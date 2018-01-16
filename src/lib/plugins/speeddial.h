/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef SPEEDDIAL_H
#define SPEEDDIAL_H

#include <QObject>
#include <QPointer>
#include <QWebEnginePage>

#include "qzcommon.h"

class QUrl;
class QPixmap;

class AutoSaver;
class PageThumbnailer;

class QUPZILLA_EXPORT SpeedDial : public QObject
{
    Q_OBJECT
public:
    struct Page {
        QString title;
        QString url;

        bool isValid() const {
            return !url.isEmpty();
        }

        bool operator==(const Page &other) {
            return (this->title == other.title &&
                    this->url == other.url);
        }
    };

    explicit SpeedDial(QObject* parent = 0);
    ~SpeedDial();

    void loadSettings();

    Page pageForUrl(const QUrl &url);
    QUrl urlForShortcut(int key);

    void addPage(const QUrl &url, const QString &title);
    void removePage(const Page &page);

    int pagesInRow();
    int sdSize();
    bool searchEnabled();
    bool sdCenter();

    QString backgroundImage();
    QString backgroundImageUrl();
    QString backgroundImageSize();
    QString searchEngine();
    QString initialScript();

signals:
    void pagesChanged();
    void thumbnailLoaded(const QString &url, const QString &src);
    void pageTitleLoaded(const QString &url, const QString &title);

public slots:
    void changed(const QString &allPages);
    void loadThumbnail(const QString &url, bool loadTitle);
    void removeImageForUrl(const QString &url);

    QStringList getOpenFileName();
    QString urlFromUserInput(const QString &url);
    void setBackgroundImage(const QString &image);
    void setBackgroundImageSize(const QString &size);
    void setPagesInRow(int count);
    void setSdSize(int count);
    void setSdCentered(bool centered);

private slots:
    void thumbnailCreated(const QPixmap &pixmap);
    void saveSettings();

private:
    QString escapeTitle(QString string) const;
    QString escapeUrl(QString url) const;

    QString generateAllPages();

    QString m_initialScript;
    QString m_thumbnailsDir;
    QString m_backgroundImage;
    QString m_backgroundImageUrl;
    QString m_backgroundImageSize;
    QString m_searchEngine;
    int m_maxPagesInRow;
    int m_sizeOfSpeedDials;
    bool m_searchEnabled;
    bool m_sdcentered;

    QList<Page> m_pages;
    AutoSaver* m_autoSaver;

    bool m_loaded;
    bool m_regenerateScript;
};

#endif // SPEEDDIAL_H
