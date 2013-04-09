/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#ifndef PAGESCREEN_H
#define PAGESCREEN_H

#include <QDialog>
#include <QFutureWatcher>

#include "qz_namespace.h"

namespace Ui
{
class PageScreen;
}

class WebView;

class QT_QUPZILLA_EXPORT PageScreen : public QDialog
{
    Q_OBJECT

public:
    explicit PageScreen(WebView* view, QWidget* parent);
    ~PageScreen();

    QImage scaleImage();

private slots:
    void createThumbnail();
    void showImage();

    void dialogAccepted();
    void saveScreen();
    void screenSaved();

private:
    void closeEvent(QCloseEvent* event);
    void createPixmap();

    Ui::PageScreen* ui;
    WebView* m_view;
    QString m_pageTitle;
    bool m_blockClose;

    QFutureWatcher<void>* m_fileSaving;
    QFutureWatcher<QImage>* m_imageScaling;
    QString m_filePath;
    QVector<QImage> m_pageImages;
};

#endif // PAGESCREEN_H
