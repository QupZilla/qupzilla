#ifndef TABWIDGET_H
#define TABWIDGET_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include "webview.h"
#include "webtab.h"
#include <QTabWidget>
#include <QTabBar>
#include <QDateTime>
#include <QToolButton>

class QupZilla;
class WebView;
class TabBar;
class WebTab;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QupZilla* mainclass, QWidget *parent = 0);
    ~TabWidget();
    enum OpenUrlIn{ CurrentTab, NewSelectedTab, NewNotSelectedTab, NewTab = NewSelectedTab };

    QByteArray saveState();
    bool restoreState(const QByteArray &state);
    void setTabText(int index, const QString& text);
    void loadSettings();

    inline TabBar* getTabBar() { return m_tabBar; }
    inline bool canRestoreTab() { return m_canRestoreTab; }


public slots:
    void closeTab(int index=-1);
    int addView(QUrl url = QUrl(), QString title = tr("New tab"), OpenUrlIn openIn = NewTab, bool selectLine = false);
    void reloadTab(int index) { weView(index)->reload(); }
    void reloadAllTabs();
    void stopTab(int index) { weView(index)->stop(); }
    void backTab(int index) { weView(index)->back(); }
    void forwardTab(int index) { weView(index)->forward(); }
    void closeAllButCurrent(int index);
    void restoreClosedTab();

private slots:
    void tabChanged(int index);
    void aboutToShowTabsMenu();
    void actionChangeIndex();

private:
    inline WebView* weView() { return qobject_cast<WebTab*>(widget(currentIndex()))->view(); }
    inline WebView* weView(int index) { return qobject_cast<WebTab*>(widget(index))->view(); }

    bool m_hideCloseButtonWithOneTab;
    bool m_hideTabBarWithOneTab;
    QUrl m_urlOnNewTab;
    QupZilla* p_QupZilla;

    bool m_canRestoreTab;
    int m_lastTabIndex;
    QUrl m_lastTabUrl;
    QByteArray m_lastTabHistory;

    TabBar* m_tabBar;

    QMenu* m_menuTabs;
    QToolButton* m_buttonAddTab;
    QToolButton* m_buttonListTabs;
};

#endif // TABWIDGET_H
