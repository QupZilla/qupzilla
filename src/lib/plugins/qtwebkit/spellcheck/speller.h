/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
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

#include <QWebElement>
#include <QStringList>
#include <QVector>
#include <QFile>

#include "qzcommon.h"

class QTextCodec;
class Hunspell;

class QMenu;
class QWebHitTestResult;

class QUPZILLA_EXPORT Speller : public QObject
{
    Q_OBJECT

public:
    struct Language {
        QString code;
        QString name;

        bool operator==(const Language &other) const {
            return this->name == other.name &&
                   this->name.left(2) == other.name.left(2);
            // Compare only first two chars of name.
            // So "cs_CZ - CzechRepublic" == "cs - CzechRepublic"
        }
    };

    explicit Speller();
    ~Speller();

    bool isEnabled() const;
    void loadSettings();

    Language language() const;
    QVector<Language> availableLanguages();

    QString dictionaryPath() const;

    void createContextMenu(QMenu* menu);
    void populateContextMenu(QMenu* menu, const QWebHitTestResult &hitTest);

    bool isMisspelled(const QString &string);
    QStringList suggest(const QString &word);

    static bool isValidWord(const QString &str);
    static Speller* instance();

public slots:
    void populateLanguagesMenu();
    void toggleEnableSpellChecking();

private slots:
    void addToDictionary();
    void replaceWord();

    void showSettings();
    void changeLanguage();

private:
    void initialize();
    void putWord(const QString &word);

    bool dictionaryExists(const QString &path) const;
    QString getDictionaryPath() const;
    QString nameForLanguage(const QString &code) const;

    QString m_dictionaryPath;
    QTextCodec* m_textCodec;
    Hunspell* m_hunspell;

    QFile m_userDictionary;
    Language m_language;
    QVector<Language> m_availableLanguages;
    bool m_enabled;

    // Replacing word
    QWebElement m_element;
    int m_startPos;
    int m_endPos;
};

// Hint to QVector to use std::realloc on item moving
Q_DECLARE_TYPEINFO(Speller::Language, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(Speller::Language)

#endif // SPELLER_H
