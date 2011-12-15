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
#include "pagescreen.h"
#include "ui_pagescreen.h"
#include "webview.h"
#include "globalfunctions.h"

PageScreen::PageScreen(WebView* view, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::PageScreen)
    , m_view(view)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    createPixmap();
    ui->label->setPixmap(m_pagePixmap);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

void PageScreen::buttonClicked(QAbstractButton* b)
{
    QString path;

    switch (ui->buttonBox->standardButton(b)) {
    case QDialogButtonBox::Cancel:
        close();
        break;

    case QDialogButtonBox::Save:
        path = QFileDialog::getSaveFileName(this, tr("Save Page Screen..."), tr("screen.png"));
        if (!path.isEmpty()) {
            m_pagePixmap.save(path);
            close();
        }
        break;

    default:
        break;
    }
}

void PageScreen::createPixmap()
{
    QWebPage* page = m_view->page();
    QSize originalSize = page->viewportSize();
    page->setViewportSize(page->mainFrame()->contentsSize());

    QImage image(page->viewportSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    page->mainFrame()->render(&painter);
    painter.end();

    m_pagePixmap = QPixmap::fromImage(image);

    page->setViewportSize(originalSize);
}

PageScreen::~PageScreen()
{
    delete ui;
}
