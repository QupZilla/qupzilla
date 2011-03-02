#ifndef TABBAR_H
#define TABBAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QTabBar>
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QStyle>
#include <QSettings>

class QupZilla;
class TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QupZilla* mainClass, QWidget *parent = 0);

signals:
    void reloadTab(int index);
    void stopTab(int index);
    void backTab(int index);
    void forwardTab(int index);
    void closeAllButCurrent(int index);
    void closeTab(int index);

public slots:

public:
    void loadSettings();

private slots:
    void contextMenuRequested(const QPoint &position);
    void reloadTab() { emit reloadTab(m_clickedTab); }
    void stopTab() { emit stopTab(m_clickedTab); }
    void backTab() { emit backTab(m_clickedTab); }
    void forwardTab() { emit forwardTab(m_clickedTab); }
    void closeAllButCurrent() { emit closeAllButCurrent(m_clickedTab); }
    void closeTab() { emit closeTab(m_clickedTab); }
    void bookmarkTab();
private:
    void mouseDoubleClickEvent(QMouseEvent *event);

    QupZilla* p_QupZilla;
    bool m_showCloseButtonWithOneTab;
    bool m_showTabBarWithOneTab;
    int m_clickedTab;

};

#endif // TABBAR_H
