/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
/****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/
#include "htmlhighlighter.h"

 HtmlHighlighter::HtmlHighlighter(QTextDocument* parent)
     : QSyntaxHighlighter(parent)
 {
     HighlightingRule rule;

     keywordFormat.setForeground(Qt::darkBlue);
     keywordFormat.setFontWeight(QFont::Bold);
     QStringList keywordPatterns;
     keywordPatterns << "(<body|</body)"
                     << "(<html|</html>|<!DOCTYPE html>|<!DOCTYPE html PUBLIC)" << "(<div|</div)" << "(<p|</p)"
                     << "(<head|</head)" << "(<meta|</meta)" << "(<title|</title)"
                     << "(<table|</table)" << "(<td|</td)" << "(<tr|</tr)"
                     << "(<span|</span)" << "(<link|</link)" << "(<script|</script)"
                     << "(<style|</style)" << "(<h1|</h1)" << "(<h2|</h2)"
                     << "(<h3|</h3)" << "(<h4|</h4" << "(<h5|</h5)"
                     << "(<ul|</ul)" << "(<li|</li)" << "(<a|</a)"
                     << "(<code|</code)" << "(<pre|</pre)" << "(<ol|</ol)"
                     << "(<b|</b)" << "(<i|</i)" << "(<col|</col)"
                     << "(<u|</u)" << "(<br)" << "(<form|</form)"
                     << "(<label|</label)" << "(<input|</input)" << "(<img|</img)"
                     << "(<center|</center)" << "(<option|</option)" << "(<select|</select)"
                     << "(<hr|</hr)" << "(<object|</object)" << "(<param|</param)"
                     << "(<tbody|</tbody)" << "(<thead|</thead)" << "(<tfoot|</tfoot)"
                     << "(<h6|</h6)" << "(<font|</font)" << "(<noscript|</noscript)"
                     << "(<embed|</embed)" << "(<base|</base)" << "(<canvas|</canvas)"
                     << "(<cufon|</cufon)" << "(<cufontext|</cufontext)" << "(<button|</button)"
                     << "(<dl|</dl)" << "(<dt|</dt)" << "(<dd|</dd)"
                     << "(<strong|</strong)" << "(<dt|</dt)" << "(<dd|</dd)"
                     << "(<em|</em)" << "(<iframe|</iframe)" << "(<th|</th)"
                     << "(<textarea|</textarea)" << "(<nav|</nav)" <<"(<section|</section)"
                     << "(<fieldset|</fieldset)" << "(<footer|</footer)" << "(<address|</address)"
                     << "(<video|</video)"
                     << "(<ol|</ol)" << "(<small|</small)" << ">";
     foreach (const QString &pattern, keywordPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = keywordFormat;
         highlightingRules.append(rule);
     }

     tagOptionsFormat.setForeground(Qt::black);
     tagOptionsFormat.setFontWeight(QFont::Bold);
     QStringList optionsPatterns;
     optionsPatterns << "type=\"" << "value=\"" << "name=\""
                     << "on(\\S{0,15})=\"" << "id=\"" << "style=\""
                     << "action=\"" << "method=\"" << "src=\""
                     << "rel=\"" << "content=\"" << "width=\""
                     << "height=\"" << "alt=\"" << "class=\""
                     << "for=\"" << "tabindex=\"" << "selected=\""
                     << "http-equiv=\"" << "media=\"" << "lang=\""
                     << "xml:lang=\"" << "dir=\"" << "accesskey=\""
                     << "target=\"" << "align=\"" << "checked=\""
                     << "language=\"" << "charset=\"" << "allowfullscreen=\""
                     << "text=\"" << "loop=\"" << "menu=\""
                     << "wmode=\"" << "classid=\"" << "border=\""
                     << "cellspacing=\"" << "cellpadding=\"" << "clear=\""
                     << "for=\"" << "tabindex=\"" << "selected=\""
                     << "frameborder=\"" << "marginwidth=\"" << "marginheight=\""
                     << "scrolling=\"" << "quality=\"" << "bgcolor=\""
                     << "allowscriptaccess=\"" << "cols=\"" << "rows=\""
                     << "profile=\"" << "colspan=\"" << "scope=\""
                     << "data=\"" << "autoplay=\"" << "hspace=\""
                     << "valign=\"" << "vspace=\""
                     << "href=\"" << "title=\"" << "xmlns=\"";
     foreach (const QString &pattern, optionsPatterns) {
         rule.pattern = QRegExp(pattern);
         rule.format = tagOptionsFormat;
         highlightingRules.append(rule);
     }

     singleLineCommentFormat.setForeground(Qt::red);
     rule.pattern = QRegExp("//[^\n]*");
     rule.format = singleLineCommentFormat;
//     highlightingRules.append(rule);

     multiLineCommentFormat.setForeground(Qt::gray);

     quotationFormat.setForeground(Qt::darkGreen);
     QRegExp rx("\".*\"");
     rx.setMinimal(true);
     rule.pattern = rx;
     rule.format = quotationFormat;
     highlightingRules.append(rule);

     functionFormat.setFontWeight(QFont::Normal);
     functionFormat.setForeground(Qt::red);
//     rule.pattern = QRegExp("(<script(.*)</script>|<style(.*)</style>)");
     rx.setPattern("<script(.*)</script>");
     rx.setMinimal(true);
     rule.pattern = rx;
     rule.format = functionFormat;
//     highlightingRules.append(rule);

     commentStartExpression = QRegExp("<!--");
     commentEndExpression = QRegExp("-->");
 }

 void HtmlHighlighter::highlightBlock(const QString &text)
 {
     foreach (const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
     setCurrentBlockState(0);

     int startIndex = 0;
     if (previousBlockState() != 1)
         startIndex = commentStartExpression.indexIn(text);

     while (startIndex >= 0) {
         int endIndex = commentEndExpression.indexIn(text, startIndex);
         int commentLength;
         if (endIndex == -1) {
             setCurrentBlockState(1);
             commentLength = text.length() - startIndex;
         } else {
             commentLength = endIndex - startIndex
                             + commentEndExpression.matchedLength();
         }
         setFormat(startIndex, commentLength, multiLineCommentFormat);
         startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
     }
 }
