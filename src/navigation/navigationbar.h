#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QSplitter>

class ToolButton;
class WebSearchBar;
class QupZilla;
class ReloadStopButton;
class NavigationBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavigationBar(QupZilla* mainClass, QWidget *parent = 0);
    ~NavigationBar();

    void showReloadButton();
    void showStopButton();

    inline ToolButton* buttonBack() { return m_buttonBack; }
    inline ToolButton* buttonNext() { return m_buttonNext; }
    inline ToolButton* buttonHome() { return m_buttonHome; }
    inline ToolButton* buttonAddTab() { return m_buttonAddTab; }
    inline ToolButton* buttonSuperMenu() { return m_supMenu; }
    inline ToolButton* buttonExitFullscreen() { return m_exitFullscreen; }
    inline WebSearchBar* searchLine() { return m_searchLine; }

signals:

public slots:
    void refreshHistory();

private slots:
    void aboutToShowHistoryNextMenu();
    void aboutToShowHistoryBackMenu();
    void goAtHistoryIndex();

private:
    QupZilla* p_QupZilla;

    QHBoxLayout* m_layout;
    QSplitter* m_navigationSplitter;
    ToolButton* m_buttonBack;
    ToolButton* m_buttonNext;
    ToolButton* m_buttonHome;
    ToolButton* m_buttonAddTab;
    ToolButton* m_supMenu;
    ToolButton* m_exitFullscreen;
    ReloadStopButton* m_reloadStop;

    QMenu* m_menuBack;
    QMenu* m_menuForward;

    WebSearchBar* m_searchLine;

};

#endif // NAVIGATIONBAR_H
