#ifndef WEBTAB_H
#define WEBTAB_H

#include <QWidget>
#include <QLayout>
#include <QPointer>
#include "webview.h"

class QupZilla;
class WebTab : public QWidget
{
    Q_OBJECT
public:
    explicit WebTab(QupZilla* mainClass, QWidget *parent = 0);
    ~WebTab();
    WebView* view() { return m_view; }

private slots:
    void showNotification(QWidget* notif);

private:
    QupZilla* p_QupZilla;
    QPointer<WebView> m_view;
    QVBoxLayout* m_layout;
};

#endif // WEBTAB_H
