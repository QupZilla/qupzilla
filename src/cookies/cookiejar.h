#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QNetworkCookieJar>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QFile>

class QupZilla;
class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QupZilla* mainClass, QObject *parent = 0);

    void loadSettings();
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);
    QList<QNetworkCookie> getAllCookies();
    void setAllCookies(const QList<QNetworkCookie> &cookieList);
    void saveCookies();
    void restoreCookies();
    void setAllowCookies(bool allow);

signals:

public slots:

private:
    QupZilla* p_QupZilla;
    bool m_allowCookies;
    bool m_filterTrackingCookie;
    bool m_allowCookiesFromDomain;
    bool m_deleteOnClose;

    QString m_activeProfil;
};

#endif // COOKIEJAR_H
