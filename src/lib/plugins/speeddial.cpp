/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#include "speeddial.h"
#include "pagethumbnailer.h"
#include "settings.h"
#include "datapaths.h"
#include "qztools.h"
#include "autosaver.h"

#include <QDir>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QWebEnginePage>
#include <QImage>

#define ENSURE_LOADED if (!m_loaded) loadSettings();

SpeedDial::SpeedDial(QObject* parent)
    : QObject(parent)
    , m_maxPagesInRow(4)
    , m_sizeOfSpeedDials(231)
    , m_sdcentered(false)
    , m_loaded(false)
    , m_regenerateScript(true)
{
    m_autoSaver = new AutoSaver(this);
    connect(m_autoSaver, SIGNAL(save()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(pagesChanged()), m_autoSaver, SLOT(changeOccurred()));
}

SpeedDial::~SpeedDial()
{
    m_autoSaver->saveIfNecessary();
}

void SpeedDial::loadSettings()
{
    m_loaded = true;

    Settings settings;
    settings.beginGroup("SpeedDial");
    QString allPages = settings.value("pages", QString()).toString();
    setBackgroundImage(settings.value("background", QString()).toString());
    m_backgroundImageSize = settings.value("backsize", "auto").toString();
    m_maxPagesInRow = settings.value("pagesrow", 4).toInt();
    m_sizeOfSpeedDials = settings.value("sdsize", 231).toInt();
    m_sdcentered = settings.value("sdcenter", false).toBool();
    settings.endGroup();

    if (allPages.isEmpty()) {
        allPages = "url:\"https://www.qupzilla.com\"|title:\"QupZilla\";"
                   "url:\"http://blog.qupzilla.com\"|title:\"QupZilla Blog\";"
                   "url:\"https://github.com/QupZilla/qupzilla\"|title:\"QupZilla GitHub\";"
                   "url:\"https://duckduckgo.com\"|title:\"DuckDuckGo\";";
    }
    changed(allPages);

    m_thumbnailsDir = DataPaths::currentProfilePath() + "/thumbnails/";

    // If needed, create thumbnails directory
    if (!QDir(m_thumbnailsDir).exists()) {
        QDir(DataPaths::currentProfilePath()).mkdir("thumbnails");
    }
}

void SpeedDial::saveSettings()
{
    ENSURE_LOADED;

    if (m_pages.isEmpty()) {
        return;
    }

    Settings settings;
    settings.beginGroup("SpeedDial");
    settings.setValue("pages", generateAllPages());
    settings.setValue("background", m_backgroundImageUrl);
    settings.setValue("backsize", m_backgroundImageSize);
    settings.setValue("pagesrow", m_maxPagesInRow);
    settings.setValue("sdsize", m_sizeOfSpeedDials);
    settings.setValue("sdcenter", m_sdcentered);
    settings.endGroup();
}

SpeedDial::Page SpeedDial::pageForUrl(const QUrl &url)
{
    ENSURE_LOADED;

    QString urlString = url.toString();
    if (urlString.endsWith(QL1C('/')))
        urlString = urlString.left(urlString.size() - 1);

    foreach (const Page &page, m_pages) {
        if (page.url == urlString) {
            return page;
        }
    }

    return Page();
}

QUrl SpeedDial::urlForShortcut(int key)
{
    ENSURE_LOADED;

    if (key < 0 || m_pages.count() <= key) {
        return QUrl();
    }

    return QUrl::fromEncoded(m_pages.at(key).url.toUtf8());
}

void SpeedDial::addPage(const QUrl &url, const QString &title)
{
    ENSURE_LOADED;

    if (url.isEmpty()) {
        return;
    }

    Page page;
    page.title = escapeTitle(title);
    page.url = escapeUrl(url.toString());

    m_pages.append(page);
    m_regenerateScript = true;

    emit pagesChanged();
}

void SpeedDial::removePage(const Page &page)
{
    ENSURE_LOADED;

    if (!page.isValid()) {
        return;
    }

    removeImageForUrl(page.url);
    m_pages.removeAll(page);
    m_regenerateScript = true;

    emit pagesChanged();
}

int SpeedDial::pagesInRow()
{
    ENSURE_LOADED;

    return m_maxPagesInRow;
}

int SpeedDial::sdSize()
{
    ENSURE_LOADED;

    return m_sizeOfSpeedDials;
}

bool SpeedDial::sdCenter()
{
    ENSURE_LOADED;

    return m_sdcentered;
}

QString SpeedDial::backgroundImage()
{
    ENSURE_LOADED;

    return m_backgroundImage;
}

QString SpeedDial::backgroundImageUrl()
{
    return m_backgroundImageUrl;
}

QString SpeedDial::backgroundImageSize()
{
    ENSURE_LOADED;

    return m_backgroundImageSize;
}

QString SpeedDial::initialScript()
{
    ENSURE_LOADED;

    if (!m_regenerateScript) {
        return m_initialScript;
    }

    m_regenerateScript = false;
    m_initialScript.clear();

    foreach (const Page &page, m_pages) {
        QString imgSource = m_thumbnailsDir + QCryptographicHash::hash(page.url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

        if (!QFile(imgSource).exists()) {
            imgSource = "qrc:html/loading.gif";

            if (!page.isValid()) {
                imgSource.clear();
            }
        }
        else {
            imgSource = QzTools::pixmapToDataUrl(QPixmap(imgSource)).toString();
        }

        m_initialScript.append(QString("addBox('%1', '%2', '%3');\n").arg(page.url, page.title, imgSource));
    }

    return m_initialScript;
}

void SpeedDial::changed(const QString &allPages)
{
    if (allPages.isEmpty()) {
        return;
    }

    const QStringList entries = allPages.split(QLatin1String("\";"), QString::SkipEmptyParts);
    m_pages.clear();

    foreach (const QString &entry, entries) {
        if (entry.isEmpty()) {
            continue;
        }

        const QStringList tmp = entry.split(QLatin1String("\"|"), QString::SkipEmptyParts);
        if (tmp.count() != 2) {
            continue;
        }

        Page page;
        page.url = tmp.at(0).mid(5);
        page.title = tmp.at(1).mid(7);

        if (page.url.endsWith(QL1C('/')))
            page.url = page.url.left(page.url.size() - 1);

        m_pages.append(page);
    }

    m_regenerateScript = true;
    emit pagesChanged();
}

void SpeedDial::loadThumbnail(const QString &url, bool loadTitle)
{
    PageThumbnailer* thumbnailer = new PageThumbnailer(this);
    thumbnailer->setUrl(QUrl::fromEncoded(url.toUtf8()));
    thumbnailer->setLoadTitle(loadTitle);
    connect(thumbnailer, SIGNAL(thumbnailCreated(QPixmap)), this, SLOT(thumbnailCreated(QPixmap)));

    thumbnailer->start();
}

void SpeedDial::removeImageForUrl(const QString &url)
{
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

    if (QFile(fileName).exists()) {
        QFile(fileName).remove();
    }
}

QStringList SpeedDial::getOpenFileName()
{
    const QString fileTypes = QString("%3(*.png *.jpg *.jpeg *.bmp *.gif *.svg *.tiff)").arg(tr("Image files"));
    const QString image = QzTools::getOpenFileName("SpeedDial-GetOpenFileName", 0, tr("Click to select image..."), QDir::homePath(), fileTypes);

    if (image.isEmpty())
        return QStringList();

    return {QzTools::pixmapToDataUrl(QPixmap(image)).toString(), QUrl::fromLocalFile(image).toEncoded()};
}

QString SpeedDial::urlFromUserInput(const QString &url)
{
    return QUrl::fromUserInput(url).toString();
}

void SpeedDial::setBackgroundImage(const QString &image)
{
    m_backgroundImage = QzTools::pixmapToDataUrl(QPixmap(QUrl(image).toLocalFile())).toString();
    m_backgroundImageUrl = image;
}

void SpeedDial::setBackgroundImageSize(const QString &size)
{
    m_backgroundImageSize = size;
}

void SpeedDial::setPagesInRow(int count)
{
    m_maxPagesInRow = count;
}

void SpeedDial::setSdSize(int count)
{
    m_sizeOfSpeedDials = count;
}

void SpeedDial::setSdCentered(bool centered)
{
    m_sdcentered = centered;

    m_autoSaver->changeOccurred();
}

void SpeedDial::thumbnailCreated(const QPixmap &pixmap)
{
    PageThumbnailer* thumbnailer = qobject_cast<PageThumbnailer*>(sender());
    if (!thumbnailer) {
        return;
    }

    bool loadTitle = thumbnailer->loadTitle();
    QString title = thumbnailer->title();
    QString url = thumbnailer->url().toString();
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

    if (pixmap.isNull()) {
        fileName = ":/html/broken-page.svg";
        title = tr("Unable to load");
    }
    else {
        if (!pixmap.save(fileName, "PNG")) {
            qWarning() << "SpeedDial::thumbnailCreated Cannot save thumbnail to " << fileName;
        }
        //fileName = QUrl::fromLocalFile(fileName).toString();
    }

    m_regenerateScript = true;
    thumbnailer->deleteLater();

    if (loadTitle)
        emit pageTitleLoaded(url, title);

    emit thumbnailLoaded(url, QzTools::pixmapToDataUrl(QPixmap(fileName)).toString());
}

QString SpeedDial::escapeTitle(QString title) const
{
    title.replace(QLatin1Char('"'), QLatin1String("&quot;"));
    title.replace(QLatin1Char('\''), QLatin1String("&apos;"));
    return title;
}

QString SpeedDial::escapeUrl(QString url) const
{
    url.remove(QLatin1Char('"'));
    url.remove(QLatin1Char('\''));
    return url;
}

QString SpeedDial::generateAllPages()
{
    QString allPages;

    foreach (const Page &page, m_pages) {
        const QString string = QString("url:\"%1\"|title:\"%2\";").arg(page.url, page.title);
        allPages.append(string);
    }

    return allPages;
}
