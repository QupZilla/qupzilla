#include "iconprovider.h"
#include "webview.h"

IconProvider::IconProvider(QObject *parent) :
    QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10*1000);
    m_timer->start();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(saveIconsToDatabase()));
}

void IconProvider::saveIcon(WebView *view)
{
    Icon item;
    item.icon = view->icon();
    item.url = view->url();

    if (item.icon.isNull())
        return;

    foreach (Icon ic, m_iconBuffer) {
        if (ic.url == item.url && ic.icon.pixmap(16,16).toImage() == item.icon.pixmap(16,16).toImage())
            return;
    }

    m_iconBuffer.append(item);
}

QIcon IconProvider::iconForUrl(const QUrl &url)
{
    foreach (Icon ic, m_iconBuffer) {
        if (ic.url == url)
            return ic.icon;
    }

    QSqlQuery query;
    query.prepare("SELECT icon FROM icons WHERE url = ?");
    query.bindValue(0, url.toEncoded(QUrl::RemoveFragment));
    query.exec();
    if (query.next()) {
        QIcon image;
        QByteArray bArray = QByteArray::fromBase64(query.value(0).toByteArray());
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        in >> image;
        buffer.close();

        if (!image.isNull())
            return image;
    }

#ifdef Q_WS_X11
    return QIcon::fromTheme("text-plain");
#else
    return QIcon(":icons/other/unknownpage.png");
#endif
}

void IconProvider::saveIconsToDatabase()
{
    foreach (Icon ic, m_iconBuffer) {
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << ic.icon;
        buffer.close();

        QSqlQuery query;
        query.prepare("SELECT id FROM icons WHERE url = ?");
        query.bindValue(0, ic.url.toEncoded(QUrl::RemoveFragment));
        query.exec();

        if (query.next())
            query.prepare("UPDATE icons SET icon = ? WHERE url = ?");
        else
            query.prepare("INSERT INTO icons (icon, url) VALUES (?,?)");

        query.bindValue(0, bArray.toBase64());
        query.bindValue(1, ic.url.toEncoded(QUrl::RemoveFragment));
        query.exec();
    }

    m_iconBuffer.clear();
}

