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
#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QTextBoundaryFinder>
#include "qwebkitplatformplugin.h"

class Speller;

class SpellCheck : public QWebSpellChecker
{
    Q_OBJECT

public:
    explicit SpellCheck();

    bool isContinousSpellCheckingEnabled() const;
    void toggleContinousSpellChecking();

    void learnWord(const QString &word);
    void ignoreWordInSpellDocument(const QString &word);
    void checkSpellingOfString(const QString &word,
                               int* misspellingLocation, int* misspellingLength);
    QString autoCorrectSuggestionForMisspelledWord(const QString &word);
    void guessesForWord(const QString &word,
                        const QString &context, QStringList &guesses);

    bool isGrammarCheckingEnabled();
    void toggleGrammarChecking();
    void checkGrammarOfString(const QString &, QList<GrammarDetail> &,
                              int* badGrammarLocation, int* badGrammarLength);

private:
    bool endOfWord(const QTextBoundaryFinder::BoundaryReasons &reasons,
                   const QTextBoundaryFinder::BoundaryType &type);
    bool startOfWord(const QTextBoundaryFinder::BoundaryReasons &reasons,
                     const QTextBoundaryFinder::BoundaryType &type);

    Speller* m_speller;

};

#endif // SPELLCHECKER_H
