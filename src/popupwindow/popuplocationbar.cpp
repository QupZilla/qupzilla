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
#include "popuplocationbar.h"
#include "popupwebview.h"
#include "toolbutton.h"

#include <QDebug>

class PopupSiteIcon : public QWidget
{
public:
    explicit PopupSiteIcon(QWidget* parent = 0) : QWidget(parent) { }
    void setIcon(const QIcon &icon) {
        m_icon = QIcon(icon.pixmap(16, 16));
        repaint();
    }

private:
    QIcon m_icon;

    void paintEvent(QPaintEvent*) {
        QPainter p(this);
        m_icon.paint(&p, rect());
    }

};

PopupLocationBar::PopupLocationBar(QWidget* parent)
    : LineEdit(parent)
    , m_view(0)
{
    m_siteIcon = new PopupSiteIcon(this);
    m_siteIcon->setIcon(QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic));
    m_siteIcon->setFixedSize(20, 26);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    setWidgetSpacing(0);

    setFixedHeight(26);
    setReadOnly(true);
}

void PopupLocationBar::setView(PopupWebView* view)
{
    m_view = view;
}

void PopupLocationBar::showUrl(const QUrl &url)
{
    setText(url.toEncoded());
    setCursorPosition(0);
}

void PopupLocationBar::showIcon()
{
    m_siteIcon->setIcon(QIcon(m_view->icon().pixmap(16, 16)));
}
