/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include <QSettings>
#include <QCryptographicHash>
#include <QDir>
#include <QWebFrame>
#include <QWeakPointer>
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
    QString loadingImagePath();

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

    QList<QWeakPointer<QWebFrame> > m_webFrames;

    bool m_loaded;
    bool m_regenerateScript;
};

#endif // SPEEDDIAL_H
