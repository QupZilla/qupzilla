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

// Based on https://gitorious.org/kdewebkit-spellcheck

#include "spellcheck.h"
#include "speller.h"
#include "mainapplication.h"

SpellCheck::SpellCheck()
    : QWebSpellChecker()
    , m_speller(mApp->speller())
{
}

bool SpellCheck::isContinousSpellCheckingEnabled() const
{
    return mApp->speller()->isEnabled();
}

void SpellCheck::toggleContinousSpellChecking()
{
}

void SpellCheck::learnWord(const QString &word)
{
    Q_UNUSED(word);
}

void SpellCheck::ignoreWordInSpellDocument(const QString &word)
{
    Q_UNUSED(word);
}

void SpellCheck::checkSpellingOfString(const QString &word,
                                       int* misspellingLocation, int* misspellingLength)
{
    if (misspellingLocation == NULL || misspellingLength == NULL) {
        return;
    }

    *misspellingLocation = -1;
    *misspellingLength = 0;

    QTextBoundaryFinder finder =  QTextBoundaryFinder(QTextBoundaryFinder::Word, word);

    QTextBoundaryFinder::BoundaryReasons boundary = finder.boundaryReasons();
    int start = finder.position(), end = finder.position();
    bool inWord = startOfWord(boundary, finder.type());

    while (finder.toNextBoundary() > 0) {
        boundary = finder.boundaryReasons();

        if (endOfWord(boundary, finder.type()) && inWord) {
            end = finder.position();
            QString str = finder.string().mid(start, end - start);
            if (Speller::isValidWord(str)) {
                if (m_speller->isMisspelled(str)) {
                    *misspellingLocation = start;
                    *misspellingLength = end - start;
                }
                return;
            }
            inWord = false;
        }

        if (startOfWord(boundary, finder.type())) {
            start = finder.position();
            inWord = true;
        }
    }
}

QString SpellCheck::autoCorrectSuggestionForMisspelledWord(const QString &word)
{
    /* Auto correcting mispelled words is really not a great idea */
    Q_UNUSED(word)
    return QString();
#if 0
    QStringList words = m_speller->suggest(word);
    if (words.size() > 0) {
        return words[0];
    }
    else {
        return QString();
    }

    return QString();
#endif
}

void SpellCheck::guessesForWord(const QString &word,
                                const QString &context, QStringList &guesses)
{
    Q_UNUSED(context);

    if (!m_speller) {
        return;
    }

    QStringList words = m_speller->suggest(word);
    guesses = words;
}

bool SpellCheck::isGrammarCheckingEnabled()
{
    return false;
}

void SpellCheck::toggleGrammarChecking()
{
}

void SpellCheck::checkGrammarOfString(const QString &, QList<GrammarDetail> &,
                                      int* badGrammarLocation, int* badGrammarLength)
{
    Q_UNUSED(badGrammarLocation);
    Q_UNUSED(badGrammarLength);
}

bool SpellCheck::endOfWord(const QTextBoundaryFinder::BoundaryReasons &reasons,
                           const QTextBoundaryFinder::BoundaryType &type)
{
#if QT_VERSION < 0x050000
    Q_UNUSED(type)
    return reasons & QTextBoundaryFinder::EndWord;
#else
    return (reasons & QTextBoundaryFinder::EndOfItem) &&
           (type & QTextBoundaryFinder::Word);
#endif
}

bool SpellCheck::startOfWord(const QTextBoundaryFinder::BoundaryReasons &reasons,
                             const QTextBoundaryFinder::BoundaryType &type)
{
#if QT_VERSION < 0x050000
    Q_UNUSED(type)
    return reasons & QTextBoundaryFinder::StartWord;
#else
    return (reasons & QTextBoundaryFinder::StartOfItem) &&
           (type & QTextBoundaryFinder::Word);
#endif
}
