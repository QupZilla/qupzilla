/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "speller.h"
#include "spellcheckdialog.h"
#include "settings.h"
#include "mainapplication.h"
#include "qztools.h"

#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include <QLocale>
#include <QDebug>
#include <QDirIterator>
#include <QWebHitTestResult>
#include <QTextBoundaryFinder>
#include <QTextStream>
#include <QMenu>

#include <hunspell/hunspell.hxx>

Speller::Speller(QObject* parent)
    : QObject(parent)
    , m_textCodec(0)
    , m_hunspell(0)
    , m_enabled(false)
{
    loadSettings();
}

bool Speller::isEnabled() const
{
    return m_enabled;
}

void Speller::loadSettings()
{

    Settings settings;
    settings.beginGroup("SpellCheck");
    m_enabled = settings.value("enabled", true).toBool();
    m_dictionaryPath = settings.value("dictionaryPath", getDictionaryPath()).toString();
    m_language.code = settings.value("language", mApp->currentLanguage()).toString();
    m_language.name = nameForLanguage(m_language.code);
    settings.endGroup();

    m_userDictionary.setFileName(mApp->currentProfilePath() + "userdictionary.txt");

    if (m_enabled) {
        initialize();
    }
}

void Speller::initialize()
{
    delete m_hunspell;
    m_hunspell = 0;

    if (m_dictionaryPath.isEmpty()) {
        qWarning() << "SpellCheck: Cannot locate dictionary path!";
        return;
    }

    QString dictionary = m_dictionaryPath + m_language.code;

    if (!dictionaryExists(dictionary)) {
        qWarning() << "SpellCheck: Dictionaries for" << dictionary << "doesn't exists!";
        return;
    }

    const QString dicPath = dictionary + ".dic";
    const QString affPath = dictionary + ".aff";

    m_hunspell = new Hunspell(affPath.toLocal8Bit().constData(),
                              dicPath .toLocal8Bit().constData());

    m_textCodec = QTextCodec::codecForName(m_hunspell->get_dic_encoding());

    if (m_userDictionary.exists()) {
        if (!m_userDictionary.open(QFile::ReadOnly)) {
            qWarning() << "SpellCheck: Cannot open" << m_userDictionary.fileName() << "for reading!";
        }
        else {
            QString word;
            QTextStream stream(&m_userDictionary);
            while (!stream.atEnd()) {
                stream >> word;
                putWord(word);
            }
        }
        m_userDictionary.close();
    }

    qDebug() << "SpellCheck: Language =" << language().code
             << (m_textCodec ? m_textCodec->name() : "invalid text codec");
}

Speller::Language Speller::language() const
{
    return m_language;
}

QList<Speller::Language> Speller::availableLanguages()
{
    if (!m_availableLanguages.isEmpty()) {
        return m_availableLanguages;
    }

    QDirIterator it(m_dictionaryPath, QStringList("*.dic"), QDir::Files);

    while (it.hasNext()) {
        const QString affFilePath = it.next().replace(QLatin1String(".dic"), QLatin1String(".aff"));

        if (!QFile(affFilePath).exists()) {
            continue;
        }

        Language lang;
        lang.code = it.fileInfo().baseName();
        lang.name = nameForLanguage(lang.code);

        if (!m_availableLanguages.contains(lang)) {
            m_availableLanguages.append(lang);
        }
    }

    return m_availableLanguages;
}

QString Speller::dictionaryPath() const
{
    return m_dictionaryPath;
}

void Speller::populateContextMenu(QMenu* menu, const QWebHitTestResult &hitTest)
{
    m_element = hitTest.element();

    if (!m_enabled || m_element.isNull() ||
            m_element.attribute(QLatin1String("type")) == QLatin1String("password")) {
        return;
    }

    const QString text = m_element.evaluateJavaScript("this.value").toString();
    const int pos = m_element.evaluateJavaScript("this.selectionStart").toInt() + 1;

    QTextBoundaryFinder finder =  QTextBoundaryFinder(QTextBoundaryFinder::Word, text);
    finder.setPosition(pos);
    m_startPos = finder.toPreviousBoundary();
    m_endPos = finder.toNextBoundary();

    const QString &word = text.mid(m_startPos, m_endPos - m_startPos).trimmed();

    if (!isValidWord(word) || !isMisspelled(word)) {
        return;
    }

    const int limit = 6;
    QStringList suggests = suggest(word);
    int count = suggests.count() > limit ? limit : suggests.count();

    QFont boldFont = menu->font();
    boldFont.setBold(true);

    for (int i = 0; i < count; ++i) {
        QAction* act = menu->addAction(suggests.at(i), this, SLOT(replaceWord()));
        act->setData(suggests.at(i));
        act->setFont(boldFont);
    }

    if (count == 0) {
        menu->addAction(tr("No suggestions"))->setEnabled(false);
    }

    menu->addAction(tr("Add to dictionary"), this, SLOT(addToDictionary()))->setData(word);
    menu->addSeparator();
}

void Speller::addToDictionary()
{
    if (QAction* act = qobject_cast<QAction*>(sender())) {
        const QString &word = act->data().toString();
        putWord(word);

        if (!m_userDictionary.open(QFile::WriteOnly | QFile::Append)) {
            qWarning() << "SpellCheck: Cannot open file" << m_userDictionary.fileName() << "for writing!";
            return;
        }

        QTextStream stream(&m_userDictionary);
        stream << word << endl;
        m_userDictionary.close();
    }
}

void Speller::replaceWord()
{
    if (m_element.isNull()) {
        return;
    }

    if (QAction* act = qobject_cast<QAction*>(sender())) {
        QString word = act->data().toString();
        QString text = m_element.evaluateJavaScript("this.value").toString();
        const int cursorPos = m_element.evaluateJavaScript("this.selectionStart").toInt();

        text.replace(m_startPos, m_endPos - m_startPos, word);
        text.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
        text.replace(QLatin1Char('\n'), QLatin1String("\\n"));
        text.replace(QLatin1Char('\''), QLatin1String("\\'"));

        m_element.evaluateJavaScript(QString("this.value='%1'").arg(text));
        m_element.evaluateJavaScript(QString("this.selectionStart=this.selectionEnd=%1").arg(cursorPos));
    }
}

void Speller::showSettings()
{
    SpellCheckDialog dialog;

    if (dialog.exec() == QDialog::Accepted) {
        loadSettings();
    }
}

void Speller::changeLanguage()
{
    QAction* act = qobject_cast<QAction*>(sender());

    if (!act) {
        return;
    }

    Language lang = act->data().value<Language>();

    Settings settings;
    settings.beginGroup("SpellCheck");
    settings.setValue("language", lang.code);
    settings.endGroup();

    loadSettings();
}

void Speller::putWord(const QString &word)
{
    if (!m_hunspell || !m_textCodec || word.isEmpty()) {
        return;
    }

    const QByteArray &data = m_textCodec->fromUnicode(word);

    if (m_hunspell->add(data.constData()) != 0) {
        qWarning() << "SpellCheck: Error while adding" << word << "word!";
    }
}

bool Speller::isMisspelled(const QString &string)
{
    if (!m_hunspell || !m_textCodec) {
        return false;
    }

    const QByteArray &data = m_textCodec->fromUnicode(string);

    return m_hunspell->spell(data.constData()) == 0;
}

QStringList Speller::suggest(const QString &word)
{
    if (!m_hunspell || !m_textCodec) {
        return QStringList();
    }

    char** suggestions;
    const QByteArray &data = m_textCodec->fromUnicode(word);
    int count = m_hunspell->suggest(&suggestions, data.constData());

    QStringList suggests;
    for (int i = 0; i < count; ++i) {
        suggests.append(m_textCodec->toUnicode(suggestions[i]));
    }
    m_hunspell->free_list(&suggestions, count);

    return suggests;
}

bool Speller::isValidWord(const QString &str)
{
    if (str.isEmpty() || (str.length() == 1 && !str[0].isLetter())) {
        return false;
    }

    const int length = str.length();

    for (int i = 0; i < length; ++i) {
        if (!str[i].isNumber()) {
            return true;
        }
    }

    return false;
}

void Speller::populateLanguagesMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());

    if (!menu || !menu->isEmpty()) {
        return;
    }

    const QList<Language> langs = availableLanguages();
    foreach(const Language & lang, langs) {
        QAction* act = menu->addAction(lang.name, this, SLOT(changeLanguage()));
        act->setCheckable(true);
        act->setChecked(m_language == lang);
        act->setData(QVariant::fromValue(lang));
    }

    if (menu->isEmpty()) {
        QAction* act = menu->addAction(tr("No suggestions"));
        act->setEnabled(false);
    }

    menu->addSeparator();
    menu->addAction(tr("Settings"), this, SLOT(showSettings()));
}

void Speller::toggleEnableSpellChecking()
{
    m_enabled = !m_enabled;

    Settings settings;
    settings.beginGroup("SpellCheck");
    settings.setValue("enabled", m_enabled);
    settings.endGroup();

    loadSettings();
}

bool Speller::dictionaryExists(const QString &path) const
{
    return QFile(path + ".dic").exists() &&
           QFile(path + ".aff").exists();
}

QString Speller::getDictionaryPath() const
{
#ifdef QZ_WS_X11
    const QString defaultDicPath = "/usr/share/hunspell/";
#else
    const QString defaultDicPath = mApp->DATADIR + "hunspell/";
#endif

    QString dicPath = QString::fromLocal8Bit(qgetenv("DICPATH")).trimmed();
    if (!dicPath.isEmpty() && !dicPath.endsWith(QLatin1Char('/'))) {
        dicPath.append(QLatin1Char('/'));
    }

    return dicPath.isEmpty() ? defaultDicPath : dicPath;
}

QString Speller::nameForLanguage(const QString &code) const
{
    QLocale loc = QLocale(code);
    QString name = QLocale::languageToString(loc.language());

    if (loc.country() != QLocale::AnyCountry) {
        name.append(" / " + QLocale::countryToString(loc.country()));
    }

    return name;
}

Speller::~Speller()
{
    delete m_hunspell;
}
