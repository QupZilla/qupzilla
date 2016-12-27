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
#ifndef PAGETHUMBNAILER_H
#define PAGETHUMBNAILER_H

#include <QObject>
#include <QSize>
#include <QUrl>

#include "qzcommon.h"

class QQuickWidget;
class QPixmap;

class QUPZILLA_EXPORT PageThumbnailer : public QObject
{
    Q_OBJECT

public:
    explicit PageThumbnailer(QObject* parent = 0);
    ~PageThumbnailer();

    void setSize(const QSize &size);

    void setUrl(const QUrl &url);
    QUrl url();

    bool loadTitle();
    void setLoadTitle(bool load);
    QString title();

    void start();

signals:
    void thumbnailCreated(const QPixmap &);

public slots:
    QString afterLoadScript() const;
    void createThumbnail(bool status);

private:
    QQuickWidget *m_view;

    QSize m_size;
    QUrl m_url;
    QString m_title;
    bool m_loadTitle;
};

#endif // PAGETHUMBNAILER_H
