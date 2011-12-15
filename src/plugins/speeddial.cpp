/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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

SpeedDial::SpeedDial(QObject* parent)
    : QObject(parent)
    , m_loaded(false)
    , m_regenerateScript(true)
{
}

void SpeedDial::loadSettings()
{
    m_loaded = true;

    QSettings settings(mApp->getActiveProfilPath() + "settings.ini", QSettings::IniFormat);
    settings.beginGroup("SpeedDial");
    m_allPages = settings.value("pages", "").toString();
    settings.endGroup();

    if (m_allPages.isEmpty()) {
        m_allPages = "url:\"http://www.google.com\"|title:\"Google\";"
                     "url:\"http://qupzilla.co.cc\"|title:\"QupZilla\";"
                     "url:\"http://qupzilla.blogspot.com\"|title:\"QupZilla Blog\";"
                     "url:\"https://github.com/nowrep/QupZilla\"|title:\"QupZilla GitHub\";"
                     "url:\"http://facebook.com\"|title:\"Facebook\";";
    }

    m_thumbnailsDir = mApp->getActiveProfilPath() + "thumbnails/";
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

QString SpeedDial::loadingImagePath()
{
    if (!m_loaded) {
        loadSettings();
    }

    return m_loadingImagePath;
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

        QString imgSource = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

        if (!QFile(imgSource).exists()) {
            loadThumbnail(url);
            imgSource = m_loadingImagePath;

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
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

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
    QString fileName = m_thumbnailsDir + QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md4).toHex() + ".png";

    if (image.isNull()) {
        fileName = "qrc:/html/broken-page.png";
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
    }

    thumbnailer->deleteLater();
}
