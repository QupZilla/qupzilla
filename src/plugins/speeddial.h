#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <QObject>
#include <QSettings>
#include <QCryptographicHash>
#include <QDir>
#include <QWebFrame>
#include <QPointer>
#include <QDebug>

class PageThumbnailer;
class SpeedDial : public QObject
{
    Q_OBJECT
public:
    explicit SpeedDial(QObject* parent = 0);

    void loadSettings();
    void saveSettings();

    void addWebFrame(QWebFrame* frame);

    QString initialScript();
    QString loadingImagePath() { return m_loadingImagePath; }

signals:

public slots:
    Q_INVOKABLE void changed(const QString &allPages);
    Q_INVOKABLE void loadThumbnail(const QString &url);
    Q_INVOKABLE void removeImageForUrl(const QString &url);

private slots:
    void thumbnailCreated(const QPixmap &image);

private:
    QString m_initialScript;
    QString m_allPages;
    QString m_thumbnailsDir;
    QString m_loadingImagePath;

    QList<QPointer<QWebFrame> > m_webFrames;

    bool m_loaded;
    bool m_regenerateScript;
};

#endif // SPEEDDIAL_H
