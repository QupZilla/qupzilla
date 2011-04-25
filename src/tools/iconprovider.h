#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#define _iconForUrl(url) mApp->iconProvider()->iconForUrl(url)

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QSqlQuery>
#include <QBuffer>
#include <QTimer>

class WebView;
class IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject* parent = 0);
    void saveIcon(WebView* view);
    QIcon iconForUrl(const QUrl &url);

signals:

public slots:
    void saveIconsToDatabase();

private:
    QTimer* m_timer;
    struct Icon {
        QUrl url;
        QIcon icon;
    };

    QList<Icon> m_iconBuffer;

};

#endif // ICONPROVIDER_H
