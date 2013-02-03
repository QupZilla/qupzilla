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
#ifndef SPELLER_H
#define SPELLER_H

#include <QStringList>

class QTextCodec;
class Hunspell;

class Speller
{
public:
    explicit Speller();
    ~Speller();

    bool initialize();

    QString backend() const;
    QString language() const;

    void learnWord(const QString &word);
    void ignoreWordInSpellDocument(const QString &word);

    bool isMisspelled(const QString &string);
    QStringList suggest(const QString &word);

private:
    bool dictionaryExists(const QString &path);
    QString parseLanguage(const QString &path);
    QString getDictionaryPath();

    static Hunspell* s_hunspell;
    static QTextCodec* s_codec;
    static QString s_dictionaryPath;
    static QString s_langugage;
    static bool s_initialized;

    QStringList m_ignoredWords;
};

#endif // SPELLER_H
