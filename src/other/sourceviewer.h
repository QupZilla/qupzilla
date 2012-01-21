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
#ifndef SOURCEVIEWER_H
#define SOURCEVIEWER_H

#include <QWidget>
#include <QBoxLayout>
#include <QTextEdit>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QWebPage>
#include <QWebFrame>
#include <QStyle>
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStatusBar>

class SourceViewer : public QWidget
{
    Q_OBJECT
public:
    explicit SourceViewer(QWebFrame* frame, const QString &selectedHtml);
    QTextEdit* sourceEdit() { return m_sourceEdit; }
signals:

public slots:

private slots:
    void save();
    void findText();
    void reload();
    void setTextEditable();
    void setTextWordWrap();
    void goToLine();

private:
    QBoxLayout* m_layout;
    QTextEdit* m_sourceEdit;
    QWeakPointer<QWebFrame> m_frame;
    QStatusBar* m_statusBar;
};

#endif // SOURCEVIEWER_H
