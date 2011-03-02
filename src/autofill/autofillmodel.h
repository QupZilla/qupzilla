#ifndef AUTOFILLMODEL_H
#define AUTOFILLMODEL_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QObject>
#include <QUrl>
#include <QNetworkRequest>

class QupZilla;
class WebView;
class AutoFillModel : public QObject
{
    Q_OBJECT
public:
    explicit AutoFillModel(QupZilla* mainClass, QObject *parent = 0);
    void completePage(WebView* view);

    bool isStored(const QUrl &url);
    bool isStoringEnabled(const QUrl &url);
    void blockStoringfor (const QUrl &url);

    QString getUsername(const QUrl &url);
    QString getPassword(const QUrl &url);
    bool addEntry(const QUrl &url, const QString &name, const QString &pass);
    bool addEntry(const QUrl &url, const QByteArray &data, const QString &pass);

    void post(const QNetworkRequest &request, const QByteArray &outgoingData);

signals:

public slots:
    void loadSettings();

private:
    QupZilla* p_QupZilla;
    QByteArray m_lastOutgoingData;
    bool m_isStoring;

};

#endif // AUTOFILLMODEL_H
