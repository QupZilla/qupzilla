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
#include "locationcompleter.h"
#include "locationcompletermodel.h"
#include "locationcompleterview.h"
#include "locationcompleterrefreshjob.h"
#include "locationbar.h"
#include "mainapplication.h"
#include "browserwindow.h"
#include "tabbedwebview.h"
#include "tabwidget.h"
#include "history.h"
#include "bookmarks.h"
#include "bookmarkitem.h"
#include "qzsettings.h"

LocationCompleterView* LocationCompleter::s_view = 0;
LocationCompleterModel* LocationCompleter::s_model = 0;

LocationCompleter::LocationCompleter(QObject* parent)
    : QObject(parent)
    , m_window(0)
    , m_locationBar(0)
    , m_lastRefreshTimestamp(0)
    , m_popupClosed(false)
{
    if (!s_view) {
        s_model = new LocationCompleterModel;
        s_view = new LocationCompleterView;
        s_view->setModel(s_model);
    }
}

void LocationCompleter::setMainWindow(BrowserWindow* window)
{
    m_window = window;
}

void LocationCompleter::setLocationBar(LocationBar* locationBar)
{
    m_locationBar = locationBar;
}

void LocationCompleter::closePopup()
{
    m_popupClosed = true;
    s_view->close();
}

void LocationCompleter::complete(const QString &string)
{
    QString trimmedStr = string.trimmed();

    // Indicates that new completion was requested by user
    // Eg. popup was not closed yet this completion session
    m_popupClosed = false;

    emit cancelRefreshJob();

    LocationCompleterRefreshJob* job = new LocationCompleterRefreshJob(trimmedStr);
    connect(job, SIGNAL(finished()), this, SLOT(refreshJobFinished()));
    connect(this, SIGNAL(cancelRefreshJob()), job, SLOT(jobCancelled()));
}

void LocationCompleter::showMostVisited()
{
    complete(QString());
}

void LocationCompleter::refreshJobFinished()
{
    LocationCompleterRefreshJob* job = qobject_cast<LocationCompleterRefreshJob*>(sender());
    Q_ASSERT(job);

    // Don't show result of older jobs
    // Also don't open the popup again when it was already closed
    if (job->timestamp() > m_lastRefreshTimestamp && !m_popupClosed) {
        s_model->setCompletions(job->completions());
        m_lastRefreshTimestamp = job->timestamp();

        showPopup();

        if (qzSettings->useInlineCompletion) {
            emit showDomainCompletion(job->domainCompletion());
        }
    }

    job->deleteLater();
}

void LocationCompleter::slotPopupClosed()
{
    disconnect(s_view, SIGNAL(closed()), this, SLOT(slotPopupClosed()));
    disconnect(s_view, SIGNAL(indexActivated(QModelIndex)), this, SLOT(indexActivated(QModelIndex)));
    disconnect(s_view, SIGNAL(indexCtrlActivated(QModelIndex)), this, SLOT(indexCtrlActivated(QModelIndex)));
    disconnect(s_view, SIGNAL(indexShiftActivated(QModelIndex)), this, SLOT(indexShiftActivated(QModelIndex)));
    disconnect(s_view, SIGNAL(indexDeleteRequested(QModelIndex)), this, SLOT(indexDeleteRequested(QModelIndex)));
    disconnect(s_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex)));

    emit popupClosed();
}

void LocationCompleter::currentChanged(const QModelIndex &index)
{
    QString completion = index.data().toString();

    if (completion.isEmpty()) {
        completion = m_originalText;
    }

    emit showCompletion(completion);
}

void LocationCompleter::indexActivated(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    const QUrl url = index.data(LocationCompleterModel::UrlRole).toUrl();
    const int tabPos = index.data(LocationCompleterModel::TabPositionTabRole).toInt();

    // Switch to tab with simple index activation
    if (tabPos > -1) {
        BrowserWindow* window = static_cast<BrowserWindow*>(index.data(LocationCompleterModel::TabPositionWindowRole).value<void*>());
        Q_ASSERT(window);
        switchToTab(window, tabPos);
        return;
    }

    if (index.data(LocationCompleterModel::BookmarkRole).toBool()) {
        BookmarkItem* bookmark = static_cast<BookmarkItem*>(index.data(LocationCompleterModel::BookmarkItemRole).value<void*>());
        bookmark->updateVisitCount();
    }

    loadUrl(url);
}

void LocationCompleter::indexCtrlActivated(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(m_window);

    if (index.data(LocationCompleterModel::BookmarkRole).toBool()) {
        BookmarkItem* bookmark = static_cast<BookmarkItem*>(index.data(LocationCompleterModel::BookmarkItemRole).value<void*>());
        bookmark->updateVisitCount();
    }

    const QUrl url = index.data(LocationCompleterModel::UrlRole).toUrl();
    const QString title = index.data(LocationCompleterModel::TitleRole).toString();

    closePopup();

    // Clear locationbar
    emit clearCompletion();

    // Open url in new tab
    m_window->tabWidget()->addView(url, title, Qz::NT_CleanSelectedTab);
}

void LocationCompleter::indexShiftActivated(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    if (index.data(LocationCompleterModel::BookmarkRole).toBool()) {
        BookmarkItem* bookmark = static_cast<BookmarkItem*>(index.data(LocationCompleterModel::BookmarkItemRole).value<void*>());
        bookmark->updateVisitCount();
    }

    const QUrl url = index.data(LocationCompleterModel::UrlRole).toUrl();
    const int tabPos = index.data(LocationCompleterModel::TabPositionTabRole).toInt();

    // Load url (instead of switching to tab) with shift activation
    if (tabPos > -1) {
        loadUrl(url);
        return;
    }

    closePopup();

    // Clear locationbar
    emit clearCompletion();

    // Open new window
    mApp->createWindow(Qz::BW_NewWindow, url);
}

void LocationCompleter::indexDeleteRequested(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    if (index.data(LocationCompleterModel::BookmarkRole).toBool()) {
        BookmarkItem* bookmark = static_cast<BookmarkItem*>(index.data(LocationCompleterModel::BookmarkItemRole).value<void*>());
        mApp->bookmarks()->removeBookmark(bookmark);
    }
    else {
        int id = index.data(LocationCompleterModel::IdRole).toInt();
        mApp->history()->deleteHistoryEntry(id);
    }

    s_view->setUpdatesEnabled(false);
    s_model->removeRow(index.row(), index.parent());
    s_view->setUpdatesEnabled(true);

    // Close popup when removing last item
    if (s_model->rowCount() == 0) {
        closePopup();
    }
    else {
        adjustPopupSize();
    }
}

void LocationCompleter::switchToTab(BrowserWindow* window, int tab)
{
    Q_ASSERT(window);
    Q_ASSERT(tab >= 0);

    closePopup();

    // Clear locationbar
    emit clearCompletion();

    TabWidget* tabWidget = window->tabWidget();

    if (window->isActiveWindow() || tabWidget->currentIndex() != tab) {
        tabWidget->setCurrentIndex(tab);
        window->show();
        window->activateWindow();
        window->raise();
    }
    else {
        window->weView()->setFocus();
    }
}

void LocationCompleter::loadUrl(const QUrl &url)
{
    closePopup();

    // Show url in locationbar
    emit showCompletion(url.toEncoded());

    // Load url
    emit loadCompletion();
}

void LocationCompleter::showPopup()
{
    Q_ASSERT(m_locationBar);

    if (s_model->rowCount() == 0) {
        s_view->close();
        return;
    }

    if (s_view->isVisible()) {
        adjustPopupSize();
        return;
    }

    QRect popupRect(m_locationBar->mapToGlobal(m_locationBar->pos()), m_locationBar->size());
    popupRect.setY(popupRect.bottom());

    s_view->setGeometry(popupRect);
    s_view->setFocusProxy(m_locationBar);
    s_view->setCurrentIndex(QModelIndex());

    connect(s_view, SIGNAL(closed()), this, SLOT(slotPopupClosed()));
    connect(s_view, SIGNAL(indexActivated(QModelIndex)), this, SLOT(indexActivated(QModelIndex)));
    connect(s_view, SIGNAL(indexCtrlActivated(QModelIndex)), this, SLOT(indexCtrlActivated(QModelIndex)));
    connect(s_view, SIGNAL(indexShiftActivated(QModelIndex)), this, SLOT(indexShiftActivated(QModelIndex)));
    connect(s_view, SIGNAL(indexDeleteRequested(QModelIndex)), this, SLOT(indexDeleteRequested(QModelIndex)));
    connect(s_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex)));

    adjustPopupSize();
}

void LocationCompleter::adjustPopupSize()
{
    const int maxItemsCount = 6;
    const int popupHeight = s_view->sizeHintForRow(0) * qMin(maxItemsCount, s_model->rowCount()) + 2 * s_view->frameWidth();

    s_view->resize(s_view->width(), popupHeight);
    s_view->show();

    m_originalText = m_locationBar->text();
}
