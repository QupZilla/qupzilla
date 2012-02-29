/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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

#include "qz_namespace.h"
#include "animatedwidget.h"

namespace Ui
{
class SourceViewerSearch;
}

class SourceViewer;

class QT_QUPZILLA_EXPORT SourceViewerSearch : public AnimatedWidget
{
    Q_OBJECT
public:
    explicit SourceViewerSearch(SourceViewer* parent = 0);

    void activateLineEdit();
    bool eventFilter(QObject* obj, QEvent* event);

signals:

public slots:

private slots:
    void next();
    void previous();
    bool find(QTextDocument::FindFlags flags);

private:
    SourceViewer* m_sourceViewer;
    Ui::SourceViewerSearch* ui;

    QString m_lastSearchedString;
};

#endif // SOURCEVIEWERSEARCH_H
