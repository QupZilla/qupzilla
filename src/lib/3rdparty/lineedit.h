#ifndef LINEEDIT_H
#define LINEEDIT_H

/**
* Copyright (c) 2008 - 2009, Benjamin C. Meyer <ben@meyerhome.net>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the Benjamin Meyer nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/
/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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

#include <QLineEdit>
#include "qz_namespace.h"

class QHBoxLayout;

/*
LineEdit is a subclass of QLineEdit that provides an easy and simple
way to add widgets on the left or right hand side of the text.

The layout of the widgets on either side are handled by a QHBoxLayout.
You can set the spacing around the widgets with setWidgetSpacing().

As widgets are added to the class they are inserted from the outside
into the center of the widget.
*/
class SideWidget;
class QT_QUPZILLA_EXPORT LineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin)

public:
    enum WidgetPosition {
        LeftSide,
        RightSide
    };

    LineEdit(QWidget* parent = 0);
    LineEdit(const QString &contents, QWidget* parent = 0);

    void addWidget(QWidget* widget, WidgetPosition position);
    void removeWidget(QWidget* widget);
    void setWidgetSpacing(int spacing);
    int widgetSpacing() const;
    int textMargin(WidgetPosition position) const;

    int leftMargin() { return m_leftMargin; }

public slots:
    void setLeftMargin(int margin);
    void updateTextMargins();

protected:
    void focusInEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    bool event(QEvent* event);

private:
    void init();

    SideWidget* m_leftWidget;
    SideWidget* m_rightWidget;
    QHBoxLayout* m_leftLayout;
    QHBoxLayout* m_rightLayout;
    QHBoxLayout* mainLayout;

    int m_leftMargin;
    bool m_ignoreMousePress;
};


class QT_QUPZILLA_EXPORT SideWidget : public QWidget
{
    Q_OBJECT

signals:
    void sizeHintChanged();

public:
    SideWidget(QWidget* parent = 0);

protected:
    bool event(QEvent* event);

};

#endif // LINEEDIT_H
