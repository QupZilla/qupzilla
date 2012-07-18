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
#include "globalfunctions.h"
#include "iconprovider.h"

#include <QMovie>
#include <QLabel>

class QT_QUPZILLA_EXPORT PopupSiteIcon : public QWidget
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
    m_siteIcon->setIcon(qIconProvider->emptyWebIcon());
    m_siteIcon->setFixedSize(20, 26);

    m_loadingAnimation = new QLabel(this);
    QMovie* movie = new QMovie(":icons/other/progress.gif", QByteArray(), m_loadingAnimation);
    m_loadingAnimation->setMovie(movie);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    addWidget(m_loadingAnimation, LineEdit::RightSide);
    setWidgetSpacing(1);

    setFixedHeight(26);
    setReadOnly(true);
}

void PopupLocationBar::setView(PopupWebView* view)
{
    m_view = view;
}

void PopupLocationBar::startLoading()
{
    m_loadingAnimation->show();
    m_loadingAnimation->movie()->start();

    updateTextMargins();
}

void PopupLocationBar::stopLoading()
{
    m_loadingAnimation->hide();
    m_loadingAnimation->movie()->stop();

    updateTextMargins();
}

void PopupLocationBar::showUrl(const QUrl &url)
{
    setText(qz_urlEncodeQueryString(url));
    setCursorPosition(0);
}

void PopupLocationBar::showIcon()
{
    m_siteIcon->setIcon(m_view->icon());
}
