/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef SOURCEVIEWERSEARCH_H
#define SOURCEVIEWERSEARCH_H

#include <QTextDocument>

#include "qzcommon.h"
#include "animatedwidget.h"

#if QTWEBENGINE_DISABLED

namespace Ui
{
class SourceViewerSearch;
}

class SourceViewer;

class QUPZILLA_EXPORT SourceViewerSearch : public AnimatedWidget
{
    Q_OBJECT
public:
    explicit SourceViewerSearch(SourceViewer* parent = 0);

    void activateLineEdit();
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void performSearch();

public slots:

private slots:
    void next();
    void previous();
    void searchWholeWords();
    void find();
    bool find(QTextDocument::FindFlags);

private:
    SourceViewer* m_sourceViewer;
    Ui::SourceViewerSearch* ui;

    QString m_lastSearchedString;
    QTextDocument::FindFlags m_findFlags;
};

#endif

#endif // SOURCEVIEWERSEARCH_H
