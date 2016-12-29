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
#ifndef LOCATIONCOMPLETER_H
#define LOCATIONCOMPLETER_H

#include <QObject>

#include "qzcommon.h"

class QUrl;
class QModelIndex;

class LocationBar;
class BrowserWindow;
class LocationCompleterModel;
class LocationCompleterView;

class QUPZILLA_EXPORT LocationCompleter : public QObject
{
    Q_OBJECT
public:
    explicit LocationCompleter(QObject* parent = 0);

    void setMainWindow(BrowserWindow* window);
    void setLocationBar(LocationBar* locationBar);

    void closePopup();

public slots:
    void complete(const QString &string);
    void showMostVisited();

signals:
    void showCompletion(const QString &completion, bool isOriginal);
    void showDomainCompletion(const QString &completion);
    void loadCompletion();
    void clearCompletion();
    void popupClosed();
    void cancelRefreshJob();

private slots:
    void refreshJobFinished();
    void slotPopupClosed();

    void currentChanged(const QModelIndex &index);

    void indexActivated(const QModelIndex &index);
    void indexCtrlActivated(const QModelIndex &index);
    void indexShiftActivated(const QModelIndex &index);
    void indexDeleteRequested(const QModelIndex &index);

private:
    void switchToTab(BrowserWindow* window, int tab);
    void loadUrl(const QUrl &url);

    void showPopup();
    void adjustPopupSize();

    BrowserWindow* m_window;
    LocationBar* m_locationBar;
    qint64 m_lastRefreshTimestamp;
    QString m_originalText;
    bool m_popupClosed;

    static LocationCompleterView* s_view;
    static LocationCompleterModel* s_model;
};

#endif // LOCATIONCOMPLETER_H
