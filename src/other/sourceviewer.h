/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QBoxLayout>
#include <QTextEdit>
#include <QApplication>

class QupZilla;
class SourceViewer : public QWidget
{
    Q_OBJECT
public:
    explicit SourceViewer(QupZilla* mainClass, QWidget *parent = 0);

signals:

public slots:

private:
    QupZilla* p_QupZilla;

    QBoxLayout* m_layout;
    QTextEdit* m_sourceEdit;

};

#endif // SOURCEVIEWER_H
