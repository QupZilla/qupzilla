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

#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include <QRegExp>
#include <QDebug>

#include <hunspell/hunspell.hxx>

Hunspell* Speller::s_hunspell = 0;
QTextCodec* Speller::s_codec = 0;
QString Speller::s_dictionaryPath;
QString Speller::s_langugage;
bool Speller::s_initialized = false;

Speller::Speller()
{
}

bool Speller::initialize()
{
    if (s_initialized) {
        return s_hunspell != 0;
    }

    s_dictionaryPath = getDictionaryPath();
    s_langugage = parseLanguage(s_dictionaryPath);

    if (s_dictionaryPath.isEmpty() || s_langugage.isEmpty()) {
        qWarning() << "SpellCheck: Initialization failed!";
        return false;
    }

    QString dicPath = s_dictionaryPath +  ".dic";
    QString affPath = s_dictionaryPath + ".aff";

    s_hunspell = new Hunspell(affPath.toLocal8Bit().constData(),
                              dicPath .toLocal8Bit().constData());

    s_codec = QTextCodec::codecForName(s_hunspell->get_dic_encoding());

    qDebug() << "SpellCheck: Language =" << language();
    s_initialized = true;
    return true;
}

QString Speller::backend() const
{
    return QString("Hunspell");
}

QString Speller::language() const
{
    return s_langugage;
}

void Speller::learnWord(const QString &word)
{
    const char* encodedWord = s_codec->fromUnicode(word).constData();
    s_hunspell->add(encodedWord);
}

void Speller::ignoreWordInSpellDocument(const QString &word)
{
    m_ignoredWords.append(word);
}

bool Speller::isMisspelled(const QString &string)
{
    if (m_ignoredWords.contains(string)) {
        return false;
    }

    const char* encodedString = s_codec->fromUnicode(string).constData();
    return s_hunspell->spell(encodedString) == 0;
}

QStringList Speller::suggest(const QString &word)
{
    char** suggestions;
    const char* encodedWord = s_codec->fromUnicode(word).constData();
    int count = s_hunspell->suggest(&suggestions, encodedWord);

    QStringList suggests;
    for (int i = 0; i < count; ++i) {
        suggests.append(s_codec->toUnicode(suggestions[i]));
    }
    s_hunspell->free_list(&suggestions, count);

    return suggests;
}

bool Speller::dictionaryExists(const QString &path)
{
    return QFile(path + ".dic").exists() &&
           QFile(path + ".aff").exists();
}

QString Speller::parseLanguage(const QString &path)
{
    if (path.contains(QLatin1Char('/'))) {
        int pos = path.lastIndexOf(QLatin1Char('/'));
        return path.mid(pos + 1);
    }
    else {
        return path;
    }
}

QString Speller::getDictionaryPath()
{
    QString dictName;
    QString defaultDicPath = "/usr/share/hunspell/";

    QString env = QString::fromLocal8Bit(qgetenv("DICTIONARY"));
    if (!env.isEmpty()) {
        if (env.contains(QLatin1Char(','))) {
            dictName = env.split(QLatin1Char(',')).first().trimmed();
        }
        else {
            dictName = env.trimmed();
        }

        if (dictName.contains(QLatin1Char('/')) && dictionaryExists(dictName)) {
            return dictName;
        }
    }

    QString dicPath = QString::fromLocal8Bit(qgetenv("DICPATH"));
    if (!dicPath.isEmpty() && !dicPath.endsWith(QLatin1Char('/'))) {
        dicPath.append(QLatin1Char('/'));
    }

    if (!dicPath.isEmpty() && dictionaryExists(dicPath + dictName)) {
        return dicPath + dictName;
    }

    if (!dictName.isEmpty()) {
        if (dictionaryExists(defaultDicPath + dictName)) {
            return defaultDicPath + dictName;
        }
    }

    QString locale = QLocale::system().name();

    if (dictionaryExists(dicPath + locale)) {
        return dicPath + locale;
    }
    else if (dictionaryExists(defaultDicPath + locale)) {
        return defaultDicPath + locale;
    }
    else {
        qWarning() << "SpellCheck: Cannot find dictionaries for" << defaultDicPath + locale;
    }

    return QString();
}

Speller::~Speller()
{
}
