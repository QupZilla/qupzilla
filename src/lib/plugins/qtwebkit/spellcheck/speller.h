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
#include <QFile>

class QTextCodec;
class Hunspell;

class Speller
{
public:
    struct Language {
        QString code;
        QString name;
    };

    explicit Speller();
    ~Speller();

    bool isEnabled() const;
    void loadSettings();

    Language language() const;
    QList<Language> availableLanguages() const;

    void learnWord(const QString &word);

    bool isMisspelled(const QString &string);
    QStringList suggest(const QString &word);

private:
    void initialize();

    bool dictionaryExists(const QString &path) const;
    QString getDictionaryPath() const;
    QString nameForLanguage(const QString &code) const;

    QString m_dictionaryPath;
    QTextCodec* m_textCodec;
    Hunspell* m_hunspell;

    QFile m_userDictionary;
    Language m_language;
    bool m_enabled;
};

#endif // SPELLER_H
