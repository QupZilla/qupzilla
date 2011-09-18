#include "qupzillaschemehandler.h"
#include "globalfunctions.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "webpage.h"

QupZillaSchemeHandler::QupZillaSchemeHandler(QObject* parent) :
    QObject(parent)
{
}

QNetworkReply* QupZillaSchemeHandler::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice* outgoingData)
{
    Q_UNUSED(outgoingData)

    if (op != QNetworkAccessManager::GetOperation)
        return 0;

    QupZillaSchemeReply* reply = new QupZillaSchemeReply(request);
    return reply;
}

QupZillaSchemeReply::QupZillaSchemeReply(const QNetworkRequest &req, QObject *parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    setUrl(req.url());

    m_pageName = req.url().path();
    if (m_pageName == "about") {
        m_buffer.open(QIODevice::ReadWrite);
        setError(QNetworkReply::NoError, tr("No Error"));

        QTimer::singleShot(0, this, SLOT(loadPage()));
        open(QIODevice::ReadOnly);
    } else {
        setError(QNetworkReply::HostNotFoundError, tr("Not Found"));
        QTimer::singleShot(0, this, SLOT(delayedFinish()));
    }
}
#include <QDebug>
void QupZillaSchemeReply::loadPage()
{
    QTextStream stream(&m_buffer);
    if (m_pageName == "about")
        stream << aboutPage();

    stream.flush();
    m_buffer.reset();

    setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("text/html"));
    setHeader(QNetworkRequest::ContentLengthHeader, m_buffer.bytesAvailable());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QByteArray("Ok"));
    emit metaDataChanged();
    emit downloadProgress(m_buffer.size(), m_buffer.size());

    emit readyRead();
    emit finished();
}

void QupZillaSchemeReply::delayedFinish()
{
    emit error(QNetworkReply::HostNotFoundError);
    emit finished();
}

qint64 QupZillaSchemeReply::bytesAvailable() const
{
    return m_buffer.bytesAvailable() + QNetworkReply::bytesAvailable();
}

qint64 QupZillaSchemeReply::readData(char *data, qint64 maxSize)
{
    return m_buffer.read(data, maxSize);
}

QString QupZillaSchemeReply::aboutPage()
{
    QString page;
    page.append(qz_readAllFileContents(":html/about.html"));
    page.replace("%FAVICON%", qz_pixmapToByteArray(QPixmap(":icons/qupzilla.png")));
    page.replace("%BOX-BORDER%", qz_pixmapToByteArray(QPixmap(":html/box-border.png")));
    page.replace("%ABOUT-IMG%", qz_pixmapToByteArray(QPixmap(":icons/other/about.png")));
    page.replace("%COPYRIGHT-INCLUDE%", qz_readAllFileContents(":html/copyright"));

    page.replace("%TITLE%", tr("About QupZilla"));
    page.replace("%ABOUT-QUPZILLA%", tr("About QupZilla"));
    page.replace("%INFORMATIONS-ABOUT-VERSION%", tr("Informations about version"));
    page.replace("%BROWSER-IDENTIFICATION%", tr("Browser Identification"));
    page.replace("%PATHS%", tr("Paths"));
    page.replace("%COPYRIGHT%", tr("Copyright"));

    QString platform;
#ifdef Q_WS_X11
    platform = tr("Linux");
#endif
#ifdef Q_WS_WIN
    platform = tr("Windows");
#endif
    page.replace("%VERSION-INFO%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Version"), QupZilla::VERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("WebKit version"), QupZilla::WEBKITVERSION) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Build time"), QupZilla::BUILDTIME) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Platform"), platform)
                 );
    page.replace("%USER-AGENT%", mApp->getWindow()->weView()->webPage()->userAgentForUrl(QUrl()));
    page.replace("%PATHS-TEXT%",
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Settings"), mApp->getActiveProfilPath() + "settings.ini") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Saved session"), mApp->getActiveProfilPath() + "session.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Pinned tabs"), mApp->getActiveProfilPath() + "pinnedtabs.dat") +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Themes"), mApp->THEMESDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Plugins"), mApp->PLUGINSDIR) +
                 QString("<dt>%1</dt><dd>%2<dd>").arg(tr("Translations"), mApp->TRANSLATIONSDIR)
                 );

    return page;
}
