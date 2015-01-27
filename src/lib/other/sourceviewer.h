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
#ifndef SOURCEVIEWER_H
#define SOURCEVIEWER_H

#if QTWEBENGINE_DISABLED

#include <QWidget>
#include <QPointer>

#include "qzcommon.h"

class PlainEditWithLines;

class QBoxLayout;
class QStatusBar;
class QWebEngineFrame;

class QUPZILLA_EXPORT SourceViewer : public QWidget
{
    Q_OBJECT
public:
    explicit SourceViewer(QWebEngineFrame* frame, const QString &selectedHtml);
    PlainEditWithLines* sourceEdit() { return m_sourceEdit; }

private slots:
    void copyAvailable(bool yes);
    void redoAvailable(bool available);
    void undoAvailable(bool available);
    void pasteAvailable();

    void loadInPage();
    void loadSource();
    void save();
    void findText();
    void reload();
    void setTextEditable();
    void setTextWordWrap();
    void goToLine();

private:
    QBoxLayout* m_layout;
    PlainEditWithLines* m_sourceEdit;
    QPointer<QWebEngineFrame> m_frame;
    QStatusBar* m_statusBar;

    QString m_selectedHtml;

    QAction* m_actionUndo;
    QAction* m_actionRedo;
    QAction* m_actionCut;
    QAction* m_actionCopy;
    QAction* m_actionPaste;
};

#endif

#endif // SOURCEVIEWER_H
