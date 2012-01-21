#ifndef POPUPWEBVIEW_H
#define POPUPWEBVIEW_H

#include <QMenu>

#include "webview.h"

class PopupWebPage;
class PopupWebView : public WebView
{
    Q_OBJECT
public:
    explicit PopupWebView(QWidget* parent = 0);

    void setWebPage(PopupWebPage* page);
    PopupWebPage* webPage();

    QWidget* overlayForJsAlert();

signals:

public slots:
    void closeView();

private slots:

private:
    void contextMenuEvent(QContextMenuEvent* event);

    PopupWebPage* m_page;
    QWebFrame* m_clickedFrame;
    QMenu* m_menu;
};

#endif // POPUPWEBVIEW_H
