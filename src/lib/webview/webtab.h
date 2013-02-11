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
#ifndef WEBTAB_H
#define WEBTAB_H

#include <QWidget>
#include <QPointer>
#include <QIcon>
#include <QUrl>

#include "qz_namespace.h"

class QVBoxLayout;
class QWebHistory;

class QupZilla;
class LocationBar;
class WebView;
class TabbedWebView;

class QT_QUPZILLA_EXPORT WebTab : public QWidget
{
    Q_OBJECT
public:
    struct SavedTab {
        QString title;
        QUrl url;
        QIcon icon;
        QByteArray history;

        SavedTab() { }
        SavedTab(WebTab* webTab);

        bool isEmpty() const { return url.isEmpty(); }
        void clear();

        friend QT_QUPZILLA_EXPORT QDataStream &operator<<(QDataStream &stream, const SavedTab &tab);
        friend QT_QUPZILLA_EXPORT QDataStream &operator>>(QDataStream &stream, SavedTab &tab);
    };

    explicit WebTab(QupZilla* mainClass, LocationBar* locationBar);
    ~WebTab();

    TabbedWebView* view() const;
    void setCurrentTab();

    QUrl url() const;
    QString title() const;
    QIcon icon() const;
    QWebHistory* history() const;

    void setHistoryData(const QByteArray &data);
    QByteArray historyData() const;

    void reload();
    void stop();
    bool isLoading() const;

    bool isPinned() const;
    void pinTab(int index);
    void setPinned(bool state);

    int tabIndex() const;
    void showNavigationBar(QWidget* bar);

    void setLocationBar(LocationBar* bar);
    LocationBar* locationBar() const;

    bool inspectorVisible() const;
    void setInspectorVisible(bool v);

    SavedTab savedTab() const;
    bool isRestored() const;

    void restoreTab(const SavedTab &tab);

    void p_restoreTab(const SavedTab &tab);
    void p_restoreTab(const QUrl &url, const QByteArray &history);

    QPixmap renderTabPreview();

    void disconnectObjects();

private slots:
    void showNotification(QWidget* notif);
    void slotRestore();

private:
    QupZilla* p_QupZilla;
    TabbedWebView* m_view;
    QVBoxLayout* m_layout;
    QWidget* m_navigationContainer;
    QPointer<LocationBar> m_locationBar;

    SavedTab m_savedTab;

    bool m_pinned;
    bool m_inspectorVisible;
};

#endif // WEBTAB_H
