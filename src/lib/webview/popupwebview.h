#ifndef POPUPWEBVIEW_H
#define POPUPWEBVIEW_H

#include "qz_namespace.h"
#include "webview.h"

class PopupWebPage;
class Menu;

class QT_QUPZILLA_EXPORT PopupWebView : public WebView
{
    Q_OBJECT
public:
    explicit PopupWebView(QWidget* parent = 0);

    void setWebPage(PopupWebPage* page);
    PopupWebPage* webPage();

    QWidget* overlayForJsAlert();
    void openUrlInNewTab(const QUrl &url, Qz::NewTabPositionFlag position);

signals:

public slots:
    void closeView();

private:
    void contextMenuEvent(QContextMenuEvent* event);

    PopupWebPage* m_page;
    Menu* m_menu;
};

#endif // POPUPWEBVIEW_H
