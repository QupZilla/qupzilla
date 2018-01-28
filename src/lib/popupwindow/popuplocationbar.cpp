/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "qztools.h"
#include "iconprovider.h"
#include "bookmarksicon.h"
#include "autofillicon.h"
#include "webpage.h"

class QUPZILLA_EXPORT PopupSiteIcon : public QWidget
{
public:
    explicit PopupSiteIcon(QWidget* parent = 0) : QWidget(parent) { }
    void setIcon(const QIcon &icon) {
        m_icon = QIcon(icon.pixmap(16));
        update();
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
    m_siteIcon->setIcon(IconProvider::emptyWebIcon());
    m_siteIcon->setFixedSize(26, 26);

    m_bookmarkIcon = new BookmarksIcon(this);
    m_autofillIcon = new AutoFillIcon(this);

    QWidget* rightSpacer = new QWidget(this);
    rightSpacer->setFixedWidth(3);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    addWidget(m_autofillIcon, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(rightSpacer, LineEdit::RightSide);
    setLeftMargin(24);

    setFixedHeight(26);
    setReadOnly(true);

    // Hide icons by default
    m_autofillIcon->hide();
}

void PopupLocationBar::setView(PopupWebView* view)
{
    m_view = view;

    m_bookmarkIcon->setWebView(m_view);
    m_autofillIcon->setWebView(m_view);
}

void PopupLocationBar::startLoading()
{
    m_autofillIcon->hide();

    updateTextMargins();
}

void PopupLocationBar::stopLoading()
{
    m_bookmarkIcon->checkBookmark(m_view->url());

    WebPage* page = qobject_cast<WebPage*>(m_view->page());

    if (page && !page->autoFillUsernames().isEmpty()) {
        m_autofillIcon->setUsernames(page->autoFillUsernames());
        m_autofillIcon->show();
    }

    updateTextMargins();
}

void PopupLocationBar::showUrl(const QUrl &url)
{
    setText(QzTools::urlEncodeQueryString(url));
    setCursorPosition(0);
}

void PopupLocationBar::showSiteIcon()
{
    m_siteIcon->setIcon(m_view->icon());
}
