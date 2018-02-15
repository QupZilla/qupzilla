/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#include "webviewtest.h"
#include "webview.h"
#include "webpage.h"

#include <QtTest/QtTest>

class TestWebView : public WebView
{
public:
    explicit TestWebView()
        : WebView()
    {
    }

    QWidget *overlayWidget() override
    {
        return this;
    }

    void closeView() override
    {
    }

    void loadInNewTab(const LoadRequest &req, Qz::NewTabPositionFlags position) override
    {
        Q_UNUSED(req)
        Q_UNUSED(position)
    }

    bool isFullScreen() override
    {
        return m_fullScreen;
    }

    void requestFullScreen(bool enable) override
    {
        m_fullScreen = enable;
    }

    bool m_fullScreen = false;
};

void WebViewTest::initTestCase()
{
}

void WebViewTest::cleanupTestCase()
{
}

void WebViewTest::loadSignalsChangePageTest()
{
    TestWebView view;
    WebPage *page1 = new WebPage;
    view.setPage(page1);

    QSignalSpy loadStartedSpy(&view, &WebView::loadStarted);
    QSignalSpy loadFinishedSpy(&view, &WebView::loadFinished);

    view.load(QUrl(":autotests/data/basic_page.html"));

    QTRY_COMPARE(loadStartedSpy.count(), 1);
    loadStartedSpy.clear();

    WebPage *page2 = new WebPage;
    view.setPage(page2);

    // WebPage: Workaround for broken load started/finished signals in QtWebEngine 5.10
    const int loadFinishedEmitCount = qstrncmp(qVersion(), "5.10.", 5) == 0 ? 2 : 1;

    QTRY_COMPARE(loadFinishedSpy.count(), loadFinishedEmitCount);
    QCOMPARE(loadStartedSpy.count(), 0);
    loadFinishedSpy.clear();

    QWebEngineView view2;
    WebPage *page3 = new WebPage;
    view2.setPage(page3);

    QSignalSpy page3LoadStart(page3, &WebPage::loadStarted);
    page3->load(QUrl(":autotests/data/basic_page.html"));
    QVERIFY(page3LoadStart.wait());

    view2.setPage(new QWebEnginePage(&view2));
    view.setPage(page3);

    QTRY_COMPARE(loadStartedSpy.count(), 1);
    QCOMPARE(loadFinishedSpy.count(), 0);
}
