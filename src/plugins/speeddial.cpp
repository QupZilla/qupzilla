#include "speeddial.h"
#include "mainapplication.h"
#include "pagethumbnailer.h"

SpeedDial::SpeedDial(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_regenerateScript(true)
{
    m_thumbnailsDir = mApp->getActiveProfilPath() + "thumbnails/";
}

void SpeedDial::loadSettings()
{
    m_loaded = true;

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SpeedDial");
    m_allPages = settings.value("pages", "url:\"http://www.google.com\"|title:\"Google\";"
                                "url:\"http://qupzilla.co.cc\"|title:\"QupZilla\";"
                                "url:\"http://qupzilla.blogspot.com\"|title:\"QupZilla Blog\";"
                                "url:\"https://github.com/nowrep/QupZilla\"|title:\"QupZilla GitHub\";"
                                "url:\"http://facebook.com\"|title:\"Facebook\";"
                               ).toString();
    settings.endGroup();

    m_loadingImagePath = "qrc:html/loading.gif";

    // If needed, create thumbnails directory
    if (!QDir(m_thumbnailsDir).exists()) {
        QDir(mApp->getActiveProfilPath()).mkdir("thumbnails");
    }
}

void SpeedDial::saveSettings()
{
    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SpeedDial");
    settings.setValue("pages", m_allPages);
    settings.endGroup();
}

void SpeedDial::addWebFrame(QWebFrame* frame)
{
    if (!m_webFrames.contains(frame)) {
        m_webFrames.append(frame);
    }
}

QString SpeedDial::initialScript()
{
    if (!m_loaded) {
        loadSettings();
    }

    if (!m_regenerateScript) {
        return m_initialScript;
    }

    m_regenerateScript = false;
    m_initialScript.clear();

    QStringList entries = m_allPages.split("\";");

    foreach(QString entry, entries) {
        if (entry.isEmpty()) {
            continue;
        }

        QStringList tmp = entry.split("\"|");
        if (tmp.count() != 2) {
            continue;
        }

        QString url = tmp.at(0).mid(5);
        QString title = tmp.at(1).mid(7);

        QString imgSource = m_thumbnailsDir + QCryptographicHash::hash(url.toAscii(), QCryptographicHash::Md4).toHex() + ".png";

        if (!QFile(imgSource).exists()) {
            loadThumbnail(url);
            imgSource = m_loadingImagePath;

            if (url.isEmpty()) {
                imgSource = "";
            }
        }
        else {
#ifdef Q_WS_X11
            imgSource.prepend("file://");
#else
            imgSource.prepend("file:///");
#endif
        }

        m_initialScript.append(QString("addBox('%1', '%2', '%3');\n").arg(url, title, imgSource));
    }

    return m_initialScript;
}

void SpeedDial::changed(const QString &allPages)
{
    m_allPages = allPages;
    m_regenerateScript = true;
}

void SpeedDial::loadThumbnail(const QString &url)
{
    if (url.isEmpty()) {
        return;
    }

    PageThumbnailer* thumbnailer = new PageThumbnailer(this);
    thumbnailer->setUrl(QUrl(url));
    connect(thumbnailer, SIGNAL(thumbnailCreated(QPixmap)), this, SLOT(thumbnailCreated(QPixmap)));

    thumbnailer->start();
}

void SpeedDial::removeImageForUrl(const QString &url)
{
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toAscii(), QCryptographicHash::Md4).toHex() + ".png";

    if (QFile(fileName).exists()) {
        QFile(fileName).remove();
    }
}

void SpeedDial::thumbnailCreated(const QPixmap &image)
{
    PageThumbnailer* thumbnailer = qobject_cast<PageThumbnailer*>(sender());
    if (!thumbnailer) {
        return;
    }

    QString url = thumbnailer->url().toString();
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toAscii(), QCryptographicHash::Md4).toHex() + ".png";

    if (!image.save(fileName)) {
        qWarning() << "SpeedDial::thumbnailCreated Cannot save thumbnail to " << fileName;
    }

    m_regenerateScript = true;
    delete thumbnailer;

    foreach(QWebFrame * frame, m_webFrames) {
        if (!frame) {
            m_webFrames.removeOne(frame);
            continue;
        }

#ifdef Q_WS_X11
            fileName.prepend("file://");
#else
            fileName.prepend("file:///");
#endif

        frame->evaluateJavaScript(QString("setImageToUrl('%1', '%2');").arg(url, fileName));
    }
}
