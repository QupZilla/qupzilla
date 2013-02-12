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
#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#include <QUrl>

#include "qz_namespace.h"
#include "lineedit.h"
#include "completer/locationcompleter.h"

class QupZilla;
class LineEdit;
class LocationCompleter;
class ClickableLabel;
class TabbedWebView;
class BookmarkIcon;
class SiteIcon;
class GoIcon;
class RssIcon;
class AutoFillIcon;

class QT_QUPZILLA_EXPORT LocationBar : public LineEdit
{
    Q_OBJECT
    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)

public:
    explicit LocationBar(QupZilla* mainClass);
    ~LocationBar();

    void setWebView(TabbedWebView* view);
    TabbedWebView* webView() { return m_webView; }

signals:
    void loadUrl(const QUrl &url);

public slots:
    void showUrl(const QUrl &url);
    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent* event);

private slots:
    void textEdit();
    void urlEnter();
    void pasteAndGo();

    void clearIcon();
    void siteIconChanged();
    void setPrivacy(bool state);
    void showRSSIcon(bool state);

    void updatePlaceHolderText();
    void showCompletion(const QString &newText);

    void onLoadStarted();
    void onLoadProgress(int progress);
    void onLoadFinished();
    void hideProgress();

    void onMessage(Qz::AppMessageType, bool);

private:
    enum ProgressStyle {
        ProgressFilled,
        ProgressBottom,
        ProgressTop
    };

    void contextMenuEvent(QContextMenuEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void dropEvent(QDropEvent* event);

    QUrl createUrl();
    QString convertUrlToText(const QUrl &url) const;

    void showGoButton();
    void hideGoButton();

    void loadSettings();

    LocationCompleter m_completer;

    BookmarkIcon* m_bookmarkIcon;
    GoIcon* m_goIcon;
    RssIcon* m_rssIcon;
    SiteIcon* m_siteIcon;
    AutoFillIcon* m_autofillIcon;

    QupZilla* p_QupZilla;
    TabbedWebView* m_webView;

    QAction* m_pasteAndGoAction;
    QAction* m_clearAction;

    bool m_rssIconVisible;
    bool m_holdingAlt;

    int m_loadProgress;
    bool m_progressVisible;
    ProgressStyle m_progressStyle;
    QColor m_progressColor;

    bool m_forceLineEditPaintEvent;
};

#endif // LOCATIONBAR_H
