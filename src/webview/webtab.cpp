#include "webtab.h"
#include "qupzilla.h"
#include "webview.h"

WebTab::WebTab(QupZilla* mainClass, QWidget *parent)
    :QWidget(parent)
    ,p_QupZilla(mainClass)
    ,m_view(0)
{
    m_layout = new QVBoxLayout(this);
    setLayout(m_layout);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);
    m_view = new WebView(p_QupZilla);
    m_layout->addWidget(m_view);

    setAutoFillBackground(true); // We don't want opaque this

    connect(m_view, SIGNAL(showNotification(QWidget*)), this, SLOT(showNotification(QWidget*)));
}

void WebTab::showNotification(QWidget *notif)
{
    if (m_layout->count() > 1)
        delete m_layout->itemAt(0)->widget();

    m_layout->insertWidget(0, notif);
    notif->show();
}

WebTab::~WebTab()
{
    delete m_view;
}
