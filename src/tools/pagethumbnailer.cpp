#include "pagethumbnailer.h"
#include "mainapplication.h"
#include "networkmanagerproxy.h"

CleanPluginFactory::CleanPluginFactory(QObject* parent)
    : QWebPluginFactory(parent)
{
}

QList<QWebPluginFactory::Plugin> CleanPluginFactory::plugins() const
{
    return QList<QWebPluginFactory::Plugin>();
}

QObject* CleanPluginFactory::create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
{
    Q_UNUSED(mimeType)
    Q_UNUSED(url)
    Q_UNUSED(argumentNames)
    Q_UNUSED(argumentValues)

    return new QObject;
}

PageThumbnailer::PageThumbnailer(QObject* parent)
    : QObject(parent)
    , m_page(new QWebPage(this))
    , m_size(QSize(231, 130))
{
    NetworkManagerProxy* m_networkProxy = new NetworkManagerProxy();
    m_networkProxy->setPrimaryNetworkAccessManager(mApp->networkManager());
    m_page->setNetworkAccessManager(m_networkProxy);

    m_page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    // HD Ready :-D
    // Every page should fit in this resolution
    m_page->setViewportSize(QSize(1280, 720));
}

void PageThumbnailer::setSize(const QSize &size)
{
    if (size.isValid()) {
        m_size = size;
    }
}

void PageThumbnailer::setUrl(const QUrl &url)
{
    if (url.isValid()) {
        m_url = url;
    }
}

QUrl PageThumbnailer::url()
{
    return m_url;
}

void PageThumbnailer::setEnableFlash(bool enable)
{
    if (!enable) {
        m_page->setPluginFactory(new CleanPluginFactory);
    }
}

void PageThumbnailer::start()
{
    m_page->mainFrame()->load(m_url);

    connect(m_page, SIGNAL(loadFinished(bool)), this, SLOT(createThumbnail()));
}

void PageThumbnailer::createThumbnail()
{
    QImage image(m_page->viewportSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    m_page->mainFrame()->render(&painter);
    painter.end();

    QImage scaledImage = image.scaled(m_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    emit thumbnailCreated(QPixmap::fromImage(scaledImage));
}

PageThumbnailer::~PageThumbnailer()
{
    delete m_page;
}
