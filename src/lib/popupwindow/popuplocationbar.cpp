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
#include "popuplocationbar.h"
#include "popupwebview.h"
#include "toolbutton.h"
#include "qztools.h"
#include "iconprovider.h"
#include "bookmarkicon.h"
#include "autofillicon.h"
#include "rssicon.h"
#include "webpage.h"

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
    m_siteIcon->setFixedSize(26, 26);

    m_bookmarkIcon = new BookmarkIcon(this);
    m_rssIcon = new RssIcon(this);
    m_autofillIcon = new AutoFillIcon(this);

    m_loadingAnimation = new QLabel(this);
    QMovie* movie = new QMovie(":icons/other/progress.gif", QByteArray(), m_loadingAnimation);
    m_loadingAnimation->setMovie(movie);
    m_loadingAnimation->setFixedSize(16, 26);

    QWidget* rightSpacer = new QWidget(this);
    rightSpacer->setFixedWidth(3);

    addWidget(m_siteIcon, LineEdit::LeftSide);
    addWidget(m_autofillIcon, LineEdit::RightSide);
    addWidget(m_bookmarkIcon, LineEdit::RightSide);
    addWidget(m_rssIcon, LineEdit::RightSide);
    addWidget(m_loadingAnimation, LineEdit::RightSide);
    addWidget(rightSpacer, LineEdit::RightSide);
    setLeftMargin(20);

    setFixedHeight(26);
    setReadOnly(true);

    // Hide icons by default
    m_rssIcon->hide();
    m_autofillIcon->hide();
}

void PopupLocationBar::setView(PopupWebView* view)
{
    m_view = view;

    m_bookmarkIcon->setWebView(m_view);
    m_rssIcon->setWebView(m_view);
    m_autofillIcon->setWebView(m_view);

    connect(m_view, SIGNAL(rssChanged(bool)), this, SLOT(showRSSIcon(bool)));
}

void PopupLocationBar::startLoading()
{
    m_loadingAnimation->show();
    m_loadingAnimation->movie()->start();

    m_autofillIcon->hide();

    updateTextMargins();
}

void PopupLocationBar::stopLoading()
{
    m_loadingAnimation->hide();
    m_loadingAnimation->movie()->stop();

    m_bookmarkIcon->checkBookmark(m_view->url());

    WebPage* page = qobject_cast<WebPage*>(m_view->page());

    if (page && page->hasMultipleUsernames()) {
        m_autofillIcon->setFormData(page->autoFillData());
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

void PopupLocationBar::showRSSIcon(bool state)
{
    m_rssIcon->setVisible(state);

    updateTextMargins();
}
