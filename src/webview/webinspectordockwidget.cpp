#include "webinspectordockwidget.h"
#include "docktitlebarwidget.h"
#include "webpage.h"
#include "webview.h"
#include "webtab.h"
#include "qupzilla.h"

WebInspectorDockWidget::WebInspectorDockWidget(QupZilla* mainClass)
    : QDockWidget()
    , p_QupZilla(mainClass)
    , m_inspector(0)
{
    setWindowTitle(tr("Web Inspector"));
    setObjectName("WebInspector");
    setFeatures(0);
    setTitleBarWidget(new DockTitleBarWidget(tr("Web Inspector"), this));

    show();
}

void WebInspectorDockWidget::close()
{
    delete m_inspector;
    p_QupZilla->weView()->webTab()->setInspectorVisible(false);

    hide();
}

void WebInspectorDockWidget::show()
{
    if (!m_inspector) {
        m_inspector = new QWebInspector(this);
        m_inspector->setPage(p_QupZilla->weView()->page());
        setWidget(m_inspector);
    }

    if (m_inspector->page() != p_QupZilla->weView()->page())
        m_inspector->setPage(p_QupZilla->weView()->page());

    p_QupZilla->weView()->webTab()->setInspectorVisible(true);

    QDockWidget::show();
}

void WebInspectorDockWidget::tabChanged()
{
    if (p_QupZilla->weView()->webTab()->inspectorVisible())
        show();
    else
        close();
}

WebInspectorDockWidget::~WebInspectorDockWidget()
{
    if (m_inspector)
        delete m_inspector;
}
