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
#include "globalfunctions.h"

#include <QFileDialog>
#include <QWebFrame>
#include <QTimer>
#include <QMovie>
#include <QPushButton>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif

static QSize limitSize(const QSize &originalSize)
{
    if (originalSize.height() > 20000) {
        return QSize(originalSize.width(), 20000);
    }

    return originalSize;
}

PageScreen::PageScreen(WebView* view, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PageScreen)
    , m_view(view)
    , m_imageScaling(0)
    , m_horizontalScrollbarSize(0)
    , m_verticalScrollbarSize(0)
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
    const QString &suggestedPath = QDir::homePath() + "/" + QString("%1.png").arg(qz_filterCharsFromFilename(m_pageTitle));
    QString path = QFileDialog::getSaveFileName(this, tr("Save Page Screen..."),
                   suggestedPath);

    if (!path.isEmpty()) {
        if (!path.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
            path.append(QLatin1String(".png"));
        }

        m_pageImage.save(path, "PNG");
        QTimer::singleShot(0, this, SLOT(close()));
    }
}

void PageScreen::createThumbnail()
{
    QWebPage* page = m_view->page();
    QSize originalSize = page->viewportSize();
    page->setViewportSize(limitSize(page->mainFrame()->contentsSize()));

    m_pageImage = QImage(page->viewportSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&m_pageImage);
    page->mainFrame()->render(&painter);
    painter.end();

    m_verticalScrollbarSize = page->mainFrame()->scrollBarGeometry(Qt::Vertical).width();
    m_horizontalScrollbarSize = page->mainFrame()->scrollBarGeometry(Qt::Horizontal).height();

    page->setViewportSize(originalSize);

    m_imageScaling = new QFutureWatcher<QImage>(this);
    connect(m_imageScaling, SIGNAL(finished()), SLOT(showImage()));

    m_imageScaling->setFuture(QtConcurrent::run(this, &PageScreen::scaleImage));
}

QImage PageScreen::scaleImage()
{
    if (m_verticalScrollbarSize > 0 || m_horizontalScrollbarSize > 0) {
        QRect newRect = m_pageImage.rect();
        newRect.setWidth(newRect.width() - m_verticalScrollbarSize);
        newRect.setHeight(newRect.height() - m_horizontalScrollbarSize);

        m_pageImage = m_pageImage.copy(newRect);
    }

    return m_pageImage.scaledToWidth(450, Qt::SmoothTransformation);
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
