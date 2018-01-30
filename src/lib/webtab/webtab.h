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
#ifndef WEBTAB_H
#define WEBTAB_H

#include <QWidget>
#include <QIcon>
#include <QUrl>

#include "qzcommon.h"

class QVBoxLayout;
class QWebEngineHistory;
class QSplitter;

class BrowserWindow;
class TabbedWebView;
class WebInspector;
class LocationBar;
class TabIcon;
class TabBar;

class QUPZILLA_EXPORT WebTab : public QWidget
{
    Q_OBJECT
public:
    struct SavedTab {
        QString title;
        QUrl url;
        QIcon icon;
        QByteArray history;
        bool isPinned;
        int zoomLevel;
        int parentTab;

        SavedTab();
        SavedTab(WebTab* webTab);

        bool isValid() const;
        void clear();

        friend QUPZILLA_EXPORT QDataStream &operator<<(QDataStream &stream, const SavedTab &tab);
        friend QUPZILLA_EXPORT QDataStream &operator>>(QDataStream &stream, SavedTab &tab);
    };

    explicit WebTab(BrowserWindow* window);
    ~WebTab();

    TabbedWebView* webView() const;
    LocationBar* locationBar() const;
    TabIcon* tabIcon() const;

    WebTab *parentTab() const;
    void setParentTab(WebTab *tab);

    QVector<WebTab*> childTabs() const;

    QUrl url() const;
    QString title(bool allowEmpty = false) const;
    QIcon icon(bool allowNull = false) const;
    QWebEngineHistory* history() const;
    int zoomLevel() const;
    void setZoomLevel(int level);

    void detach();
    void attach(BrowserWindow* window);

    QByteArray historyData() const;

    void stop();
    void reload();
    void unload();
    bool isLoading() const;

    bool isPinned() const;
    void setPinned(bool state);
    void togglePinned();

    bool isMuted() const;
    void setMuted(bool muted);
    void toggleMuted();

    int tabIndex() const;

    bool isCurrentTab() const;

    bool haveInspector() const;
    void showWebInspector(bool inspectElement = false);
    void toggleWebInspector();

    void showSearchToolBar();

    bool isRestored() const;
    void restoreTab(const SavedTab &tab);
    void p_restoreTab(const SavedTab &tab);
    void p_restoreTab(const QUrl &url, const QByteArray &history, int zoomLevel);

    void tabActivated();

private slots:
    void showNotification(QWidget* notif);
    void loadStarted();
    void loadFinished();

signals:
    void titleChanged(const QString &title);
    void iconChanged(const QIcon &icon);
    void pinnedChanged(bool pinned);
    void restoredChanged(bool restored);
    void parentTabChanged(WebTab *tab);
    void childTabAdded(WebTab *tab);
    void childTabRemoved(WebTab *tab);

private:
    void titleWasChanged(const QString &title);
    void resizeEvent(QResizeEvent *event) override;

    BrowserWindow* m_window;
    QVBoxLayout* m_layout;
    QSplitter* m_splitter;

    TabbedWebView* m_webView;
    WebInspector* m_inspector;
    LocationBar* m_locationBar;
    TabIcon* m_tabIcon;
    TabBar* m_tabBar;
    QWidget *m_notificationWidget;

    WebTab *m_parentTab = nullptr;
    QVector<WebTab*> m_childTabs;

    SavedTab m_savedTab;
    bool m_isPinned;
};

#endif // WEBTAB_H
