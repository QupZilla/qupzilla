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
#ifndef POPUPWINDOW_H
#define POPUPWINDOW_H

#include <QWidget>

#include "qz_namespace.h"

class QVBoxLayout;
class QStatusBar;

class PopupWebView;
class PopupWebPage;
class PopupStatusBarMessage;
class PopupLocationBar;
class ProgressBar;

class QT_QUPZILLA_EXPORT PopupWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PopupWindow(PopupWebView* view, bool showStatusBar);

    QStatusBar* statusBar();
    PopupWebView* webView();

signals:

public slots:
    void setWindowGeometry(const QRect &newRect);
    void setStatusBarVisibility(bool visible);
    void setMenuBarVisibility(bool visible);
    void setToolBarVisibility(bool visible);

private slots:
    void titleChanged();
    void showNotification(QWidget* notif);
    void showStatusBarMessage(const QString &message);

    void loadStarted();
    void loadProgress(int value);
    void loadFinished();

private:
    void closeEvent(QCloseEvent* event);

    PopupWebView* m_view;
    PopupWebPage* m_page;
    PopupLocationBar* m_locationBar;
    PopupStatusBarMessage* m_statusBarMessage;
    ProgressBar* m_progressBar;

    QVBoxLayout* m_layout;
    QStatusBar* m_statusBar;

};

#endif // POPUPWINDOW_H
