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
#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#include "qzcommon.h"
#include "lineedit.h"

class QStringListModel;

class BrowserWindow;
class LocationCompleter;
class ClickableLabel;
class TabbedWebView;
class BookmarksIcon;
class SiteIcon;
class GoIcon;
class RssIcon;
class AutoFillIcon;
class LoadRequest;

class QUPZILLA_EXPORT LocationBar : public LineEdit
{
    Q_OBJECT

public:
    explicit LocationBar(BrowserWindow* window);

    TabbedWebView* webView() const;
    void setWebView(TabbedWebView* view);

    static QString convertUrlToText(const QUrl &url);

public slots:
    void setText(const QString &text);
    void showUrl(const QUrl &url);

private slots:
    void textEdited(const QString &text);
    void requestLoadUrl();
    void pasteAndGo();

    void updateSiteIcon();
    void updatePlaceHolderText();

    void setPrivacyState(bool state);
    void setRssIconVisible(bool state);
    void setGoIconVisible(bool state);

    void showCompletion(const QString &completion);
    void showDomainCompletion(const QString &completion);
    void clearCompletion();

    void loadStarted();
    void loadProgress(int progress);
    void loadFinished();
    void hideProgress();

    void loadSettings();

private:
    enum ProgressStyle {
        ProgressFilled,
        ProgressBottom,
        ProgressTop
    };

    void contextMenuEvent(QContextMenuEvent* event);
    void showEvent(QShowEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void dropEvent(QDropEvent* event);
    void paintEvent(QPaintEvent* event);

    LoadRequest createLoadRequest() const;
    void refreshTextFormat();

    LocationCompleter* m_completer;
    QStringListModel* m_domainCompleterModel;

    BookmarksIcon* m_bookmarkIcon;
    GoIcon* m_goIcon;
    RssIcon* m_rssIcon;
    SiteIcon* m_siteIcon;
    AutoFillIcon* m_autofillIcon;

    BrowserWindow* m_window;
    TabbedWebView* m_webView;

    QAction* m_pasteAndGoAction;
    QAction* m_clearAction;

    bool m_rssIconVisible;
    bool m_holdingAlt;
    int m_oldTextLength;
    int m_currentTextLength;

    int m_loadProgress;
    bool m_progressVisible;
    ProgressStyle m_progressStyle;
    QColor m_progressColor;
};

#endif // LOCATIONBAR_H
