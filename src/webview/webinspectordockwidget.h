#ifndef WEBINSPECTORDOCKWIDGET_H
#define WEBINSPECTORDOCKWIDGET_H

#include <QDockWidget>
#include <QWebInspector>
#include <QPair>
#include <QPointer>

class WebPage;
class QupZilla;
class WebInspectorDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit WebInspectorDockWidget(QupZilla* mainClass);
    ~WebInspectorDockWidget();

    void setPage(WebPage* page) { m_page = page; }

signals:

public slots:
    void tabChanged();

    void close();
    void show();

private:
    QupZilla* p_QupZilla;
    QPointer<QWebInspector> m_inspector;
    WebPage* m_page;
};

#endif // WEBINSPECTORDOCKWIDGET_H
