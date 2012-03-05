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
#include "globalfunctions.h"

#include <QFileDialog>
#include <QWebFrame>
#include <QTimer>
#include <QMovie>
#include <QtConcurrentRun>
#include <QPushButton>

QImage scale(QImage image)
{
    return image.scaled(QSize(470, 370), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

PageScreen::PageScreen(WebView* view, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PageScreen)
    , m_view(view)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QMovie* mov = new QMovie(":html/loading.gif");
    ui->label->setMovie(mov);
    mov->start();

    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(dialogAccepted()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

    QTimer::singleShot(200, this, SLOT(createThumbnail()));
}

void PageScreen::dialogAccepted()
{
    const QString &path = QFileDialog::getSaveFileName(this, tr("Save Page Screen..."), tr("screen.png"));

    if (!path.isEmpty()) {
        m_pageImage.save(path);
        close();
    }
}

void PageScreen::createThumbnail()
{
    QWebPage* page = m_view->page();
    QSize originalSize = page->viewportSize();
    page->setViewportSize(page->mainFrame()->contentsSize());

    m_pageImage = QImage(page->viewportSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&m_pageImage);
    page->mainFrame()->render(&painter);
    painter.end();

    page->setViewportSize(originalSize);

    m_imageScaling = new QFutureWatcher<QImage>(this);
    connect(m_imageScaling, SIGNAL(finished()), SLOT(showImage()));

    m_imageScaling->setFuture(QtConcurrent::run(scale, m_pageImage));
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
