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
#include "pagescreen.h"
#include "ui_pagescreen.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "qztools.h"

#include <QFileDialog>
#include <QWebFrame>
#include <QLabel>
#include <QTimer>
#include <QMovie>
#include <QPushButton>
#include <QCloseEvent>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif

PageScreen::PageScreen(WebView* view, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PageScreen)
    , m_view(view)
    , m_blockClose(false)
    , m_fileSaving(0)
    , m_imageScaling(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QMovie* mov = new QMovie(":html/loading.gif");
    ui->label->setMovie(mov);
    mov->start();

    m_pageTitle = m_view->title();

    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(dialogAccepted()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

    QTimer::singleShot(200, this, SLOT(createThumbnail()));
}

void PageScreen::dialogAccepted()
{
    const QString &suggestedPath = QString("%1/%2.png").arg(QDir::homePath(),
                                   QzTools::filterCharsFromFilename(m_pageTitle));
    m_filePath = QFileDialog::getSaveFileName(this, tr("Save Page Screen..."), suggestedPath);

    if (!m_filePath.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_blockClose = true;

        m_fileSaving = new QFutureWatcher<void>(this);
        m_fileSaving->setFuture(QtConcurrent::run(this, &PageScreen::saveScreen));
        connect(m_fileSaving, SIGNAL(finished()), SLOT(screenSaved()));
    }
}

void PageScreen::saveScreen()
{
    QString pathWithoutSuffix = m_filePath;
    if (pathWithoutSuffix.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
        pathWithoutSuffix = pathWithoutSuffix.mid(0, pathWithoutSuffix.length() - 4);
    }

    if (m_pageImages.count() == 1) {
        m_pageImages.first().save(pathWithoutSuffix + ".png", "PNG");
    }
    else {
        int part = 1;
        foreach(const QImage & image, m_pageImages) {
            const QString &fileName = pathWithoutSuffix + ".part" + QString::number(part);
            image.save(fileName + ".png", "PNG");
            part++;
        }
    }

    m_blockClose = false;
}

void PageScreen::screenSaved()
{
    QApplication::restoreOverrideCursor();
}

void PageScreen::closeEvent(QCloseEvent* event)
{
    if (m_blockClose) {
        event->ignore();
        return;
    }

    QDialog::closeEvent(event);
}

void PageScreen::createThumbnail()
{
    QWebPage* page = m_view->page();

    const int heightLimit = 20000;
    const QPoint originalScrollPosition = page->mainFrame()->scrollPosition();
    const QSize &originalSize = page->viewportSize();
    const QSize &frameSize = page->mainFrame()->contentsSize();
    const int verticalScrollbarSize = page->mainFrame()->scrollBarGeometry(Qt::Vertical).width();
    const int horizontalScrollbarSize = page->mainFrame()->scrollBarGeometry(Qt::Horizontal).height();

    int yPosition = 0;
    bool canScroll = frameSize.height() > heightLimit;

    /* We will split rendering page into smaller parts to avoid infinite loops
     * or crashes.
     */
    do {
        int remainingHeight = frameSize.height() - yPosition;
        if (remainingHeight <= 0) {
            break;
        }

        QSize size(frameSize.width(),
                   remainingHeight > heightLimit ? heightLimit : remainingHeight);
        page->setViewportSize(size);
        page->mainFrame()->scroll(0, qMax(0, yPosition - horizontalScrollbarSize));

        QImage image(page->viewportSize().width() - verticalScrollbarSize,
                     page->viewportSize().height() - horizontalScrollbarSize,
                     QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&image);
        page->mainFrame()->render(&painter);
        painter.end();

        m_pageImages.append(image);

        canScroll = remainingHeight > heightLimit;
        yPosition += size.height();
    }
    while (canScroll);

    page->setViewportSize(originalSize);
    page->mainFrame()->setScrollBarValue(Qt::Vertical, originalScrollPosition.y());
    page->mainFrame()->setScrollBarValue(Qt::Horizontal, originalScrollPosition.x());

    m_imageScaling = new QFutureWatcher<QImage>(this);
    m_imageScaling->setFuture(QtConcurrent::run(this, &PageScreen::scaleImage));
    connect(m_imageScaling, SIGNAL(finished()), SLOT(showImage()));
}

QImage PageScreen::scaleImage()
{
    QList<QImage> scaledImages;
    int sumHeight = 0;

    foreach(const QImage & image, m_pageImages) {
        QImage scaled = image.scaledToWidth(450, Qt::SmoothTransformation);

        scaledImages.append(scaled);
        sumHeight += scaled.height();
    }

    QImage finalImage(QSize(450, sumHeight), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&finalImage);

    int offset = 0;
    foreach(const QImage & image, scaledImages) {
        painter.drawImage(0, offset, image);
        offset += image.height();
    }

    return finalImage;
}

void PageScreen::showImage()
{
    delete ui->label->movie();

    ui->label->setPixmap(QPixmap::fromImage(m_imageScaling->result()));
}

PageScreen::~PageScreen()
{
    delete ui;
}
