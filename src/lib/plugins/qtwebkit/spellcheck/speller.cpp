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

#include <hunspell/hunspell.hxx>

Speller::Speller()
    : m_textCodec(0)
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
    initialize();
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
        m_hunspell->add_dic(m_userDictionary.fileName().toLocal8Bit().constData());
    }

    qDebug() << "SpellCheck: Language =" << language().code;
}

Speller::Language Speller::language() const
{
    return m_language;
}

QList<Speller::Language> Speller::availableLanguages() const
{
    QList<Language> languages;

    QDirIterator it(m_dictionaryPath, QStringList("*.dic"),
                    QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);

    while (it.hasNext()) {
        const QString affFilePath = it.next().replace(QLatin1String(".dic"), QLatin1String(".aff"));

        if (!QFile(affFilePath).exists()) {
            continue;
        }

        Language lang;
        lang.code = it.fileInfo().baseName();
        lang.name = nameForLanguage(lang.code);

        languages.append(lang);
    }

    return languages;
}

void Speller::learnWord(const QString &word)
{
    if (!m_hunspell || !m_textCodec) {
        return;
    }

    const char* encodedWord = m_textCodec->fromUnicode(word).constData();
    m_hunspell->add(encodedWord);

    if (!m_userDictionary.open(QFile::WriteOnly | QFile::Append)) {
        qWarning() << "SpellCheck: Cannot open file" << m_userDictionary.fileName() << "for writing!";
        return;
    }

    m_userDictionary.write(word.toUtf8());
    m_userDictionary.write("\n");
    m_userDictionary.close();
}

bool Speller::isMisspelled(const QString &string)
{
    if (!m_hunspell || !m_textCodec) {
        return false;
    }

    const char* encodedString = m_textCodec->fromUnicode(string).constData();
    return m_hunspell->spell(encodedString) == 0;
}

QStringList Speller::suggest(const QString &word)
{
    if (!m_hunspell || !m_textCodec) {
        return QStringList();
    }

    char** suggestions;
    const char* encodedWord = m_textCodec->fromUnicode(word).constData();
    int count = m_hunspell->suggest(&suggestions, encodedWord);

    QStringList suggests;
    for (int i = 0; i < count; ++i) {
        suggests.append(m_textCodec->toUnicode(suggestions[i]));
    }
    m_hunspell->free_list(&suggestions, count);

    return suggests;
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
