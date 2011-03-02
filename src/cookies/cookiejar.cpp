#include "cookiejar.h"
#include "qupzilla.h"
#define COOKIE_DEBUG

//TODO: black/white listing
CookieJar::CookieJar(QupZilla* mainClass, QObject *parent) :
    QNetworkCookieJar(parent)
    ,p_QupZilla(mainClass)
{
    loadSettings();
//    activeProfil = MainApplication::getInstance()->getActiveProfil();
    m_activeProfil = MainApplication::getInstance()->getActiveProfil();
}

void CookieJar::loadSettings()
{
    QSettings settings(m_activeProfil+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_allowCookies = settings.value("allowCookies",true).toBool();
    m_allowCookiesFromDomain = settings.value("allowCookiesFromVisitedDomainOnly",false).toBool();
    m_filterTrackingCookie = settings.value("filterTrackingCookie",false).toBool();
    m_deleteOnClose = settings.value("deleteCookiesOnClose", false).toBool();
}

void CookieJar::setAllowCookies(bool allow)
{
    m_allowCookies = allow;
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!m_allowCookies)
        return QNetworkCookieJar::setCookiesFromUrl(QList<QNetworkCookie>(), url);

    QList<QNetworkCookie> newList = cookieList;
    QDateTime now = QDateTime::currentDateTime();

    foreach (QNetworkCookie cok, newList) {
        if (m_allowCookiesFromDomain && !url.toString().contains(cok.domain())) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged for domain mismatch" << cok;
#endif
            newList.removeOne(cok);
            continue;
        }
        if (m_filterTrackingCookie && cok.expirationDate() > now.addYears(2)) {
#ifdef COOKIE_DEBUG
            qDebug() << "purged as tracking " << cok;
#endif
            newList.removeOne(cok);
            continue;
        }
    }
    return QNetworkCookieJar::setCookiesFromUrl(newList, url);
}

void CookieJar::saveCookies()
{
    if (m_deleteOnClose)
        return;

    QList<QNetworkCookie> allCookies = getAllCookies();
    QFile file(m_activeProfil+"cookies.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    int count = allCookies.count();

    stream << count;
    for (int i = 0; i<count; i++) {
        stream << allCookies.at(i).toRawForm();
    }

    file.close();
}

void CookieJar::restoreCookies()
{
    if (!QFile::exists(m_activeProfil+"cookies.dat"))
        return;
    QDateTime now = QDateTime::currentDateTime();

    QList<QNetworkCookie> restoredCookies;
    QFile file(m_activeProfil+"cookies.dat");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int count;

    stream >> count;
    for (int i = 0; i<count; i++) {
        QByteArray rawForm;
        stream >> rawForm;
        QNetworkCookie cok = QNetworkCookie::parseCookies(rawForm).at(0);
        if (cok.expirationDate() < now)
            continue;
        if (cok.isSessionCookie())
            continue;
        restoredCookies.append(cok);
    }

    file.close();
    setAllCookies(restoredCookies);
}

QList<QNetworkCookie> CookieJar::getAllCookies()
{
    return QNetworkCookieJar::allCookies();
}

void CookieJar::setAllCookies(const QList<QNetworkCookie> &cookieList)
{
    QNetworkCookieJar::setAllCookies(cookieList);
}
