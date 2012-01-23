/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "mainapplication.h"
#include "pagethumbnailer.h"
#include "settings.h"

SpeedDial::SpeedDial(QObject* parent)
    : QObject(parent)
    , m_maxPagesInRow(4)
    , m_sizeOfSpeedDials(231)
    , m_loaded(false)
    , m_regenerateScript(true)
{
}

void SpeedDial::loadSettings()
{
    m_loaded = true;

    Settings settings;
    settings.beginGroup("SpeedDial");
    m_allPages = settings.value("pages", "").toString();
    m_backgroundImage = settings.value("background", "").toString();
    m_backgroundImageSize = settings.value("backsize", "auto").toString();
    m_maxPagesInRow = settings.value("pagesrow", 4).toInt();
    m_sizeOfSpeedDials = settings.value("sdsize", 231).toInt();
    settings.endGroup();

    if (m_allPages.isEmpty()) {
        m_allPages = "url:\"http://www.google.com\"|title:\"Google\";"
                     "url:\"http://www.qupzilla.com\"|title:\"QupZilla\";"
                     "url:\"http://blog.qupzilla.com\"|title:\"QupZilla Blog\";"
                     "url:\"https://github.com/nowrep/QupZilla\"|title:\"QupZilla GitHub\";"
                     "url:\"http://facebook.com\"|title:\"Facebook\";";
    }

    m_thumbnailsDir = mApp->getActiveProfilPath() + "thumbnails/";

    // If needed, create thumbnails directory
    if (!QDir(m_thumbnailsDir).exists()) {
        QDir(mApp->getActiveProfilPath()).mkdir("thumbnails");
    }
}

void SpeedDial::saveSettings()
{
    if (m_allPages.isEmpty()) {
        return;
    }

    Settings settings;
    settings.beginGroup("SpeedDial");
    settings.setValue("pages", m_allPages);
    settings.setValue("background", m_backgroundImage);
    settings.setValue("backsize", m_backgroundImageSize);
    settings.setValue("pagesrow", m_maxPagesInRow);
    settings.setValue("sdsize", m_sizeOfSpeedDials);
    settings.endGroup();
}

void SpeedDial::addWebFrame(QWebFrame* frame)
{
    if (!m_webFrames.contains(frame)) {
        m_webFrames.append(frame);
    }
}

void SpeedDial::addPage(const QUrl &url, const QString &title)
{
    if (url.isEmpty()) {
        return;
    }

    QString sitePair = QString("url:\"%1\"|title:\"%2\";").arg(url.toString(), title);

    m_allPages.append(sitePair);

    for (int i = 0; i < m_webFrames.count(); i++) {
        QWebFrame* frame = m_webFrames.at(i).data();
        if (!frame) {
            m_webFrames.removeAt(i);
            i--;
            continue;
        }

        frame->page()->triggerAction(QWebPage::Reload);
    }
}

int SpeedDial::pagesInRow()
{
    if (!m_loaded) {
        loadSettings();
    }

    return m_maxPagesInRow;
}

int SpeedDial::sdSize()
{
    if (!m_loaded) {
        loadSettings();
    }

    return m_sizeOfSpeedDials;
}

QString SpeedDial::backgroundImage()
{
    if (!m_loaded) {
        loadSettings();
    }

    return m_backgroundImage;
}

QString SpeedDial::backgroundImageSize()
{
    if (!m_loaded) {
        loadSettings();
    }

    return m_backgroundImageSize;
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

    foreach(const QString &entry, entries) {
        if (entry.isEmpty()) {
            continue;
        }

        QStringList tmp = entry.split("\"|");
        if (tmp.count() != 2) {
            continue;
        }

        QString url = tmp.at(0).mid(5);
        QString title = tmp.at(1).mid(7);

        QString imgSource = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

        if (!QFile(imgSource).exists()) {
            loadThumbnail(url);
            imgSource = "qrc:html/loading.gif";

            if (url.isEmpty()) {
                imgSource = "";
            }
        }
        else {
            imgSource = QUrl::fromLocalFile(imgSource).toString();
        }

        m_initialScript.append(QString("addBox('%1', '%2', '%3');\n").arg(url, title, imgSource));
    }

    return m_initialScript;
}

void SpeedDial::changed(const QString &allPages)
{
    if (allPages.isEmpty()) {
        return;
    }

    m_allPages = allPages;
    m_regenerateScript = true;
}

void SpeedDial::loadThumbnail(const QString &url, bool loadTitle)
{
    if (url.isEmpty()) {
        return;
    }

    PageThumbnailer* thumbnailer = new PageThumbnailer(this);
    thumbnailer->setUrl(QUrl::fromUserInput(url));
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

QString SpeedDial::getOpenFileName()
{
    return QFileDialog::getOpenFileName(0, tr("Select image..."), QDir::homePath(), "(*.png *.jpg *.jpeg *.bmp *.gif *.tiff)");
}

void SpeedDial::setBackgroundImage(const QString &image)
{
    m_backgroundImage = image;
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

void SpeedDial::thumbnailCreated(const QPixmap &image)
{
    PageThumbnailer* thumbnailer = qobject_cast<PageThumbnailer*>(sender());
    if (!thumbnailer) {
        return;
    }

    bool loadTitle = thumbnailer->loadTitle();
    QString title = thumbnailer->title();
    QString url = thumbnailer->url().toString();
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

    if (image.isNull()) {
        fileName = "qrc:/html/broken-page.png";
        title = tr("Unable to load");
        loadTitle = true;
    }
    else {
        if (!image.save(fileName)) {
            qWarning() << "SpeedDial::thumbnailCreated Cannot save thumbnail to " << fileName;
        }

        fileName = QUrl::fromLocalFile(fileName).toString();
    }

    m_regenerateScript = true;

    for (int i = 0; i < m_webFrames.count(); i++) {
        QWebFrame* frame = m_webFrames.at(i).data();
        if (!frame) {
            m_webFrames.removeAt(i);
            i--;
            continue;
        }

        frame->evaluateJavaScript(QString("setImageToUrl('%1', '%2');").arg(url, fileName));
        if (loadTitle) {
            frame->evaluateJavaScript(QString("setTitleToUrl('%1', '%2');").arg(url, title));
        }
    }

    thumbnailer->deleteLater();
}
