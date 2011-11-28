#ifndef PAGETHUMBNAILER_H
#define PAGETHUMBNAILER_H

#include <QObject>
#include <QWebPage>
#include <QWebFrame>
#include <QPainter>
#include <QWebPluginFactory>

class CleanPluginFactory : public QWebPluginFactory
{
    Q_OBJECT
public:
    explicit CleanPluginFactory(QObject* parent = 0);

    QList<QWebPluginFactory::Plugin> plugins() const;
    QObject* create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const;
};

class PageThumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit PageThumbnailer(QObject* parent = 0);

    void setSize(const QSize &size);
    void setUrl(const QUrl &url);

    void start();

signals:
    void thumbnailCreated(QPixmap);

public slots:

private slots:
    void createThumbnail();

private:
    QWebPage* m_page;

    QSize m_size;
    QUrl m_url;
};

#endif // PAGETHUMBNAILER_H
