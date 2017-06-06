/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "downloadoptionsdialog.h"
#include "ui_downloadoptionsdialog.h"
#include "iconprovider.h"

#include <QClipboard>
#include <QMimeDatabase>
#include <QWebEngineDownloadItem>

DownloadOptionsDialog::DownloadOptionsDialog(const QString &fileName, QWebEngineDownloadItem *downloadItem, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DownloadOptionsDialog)
    , m_downloadItem(downloadItem)
    , m_signalEmited(false)
{
    ui->setupUi(this);

    ui->fileName->setText("<b>" + fileName + "</b>");
    ui->fromServer->setText(m_downloadItem->url().host());

    const QIcon fileIcon = IconProvider::instance()->standardIcon(QStyle::SP_FileIcon);

    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForName(downloadItem->mimeType());
    if (mime.isValid() && !mime.isDefault()) {
        ui->mimeName->setText(mime.comment());
        ui->iconLabel->setPixmap(QIcon::fromTheme(mime.iconName(), fileIcon).pixmap(22));
    } else {
        ui->mimeFrame->hide();
        ui->iconLabel->setPixmap(fileIcon.pixmap(22));
    }

    setWindowTitle(tr("Opening %1").arg(fileName));

    ui->buttonBox->setFocus();

    connect(ui->copyDownloadLink, SIGNAL(clicked(QPoint)), this, SLOT(copyDownloadLink()));
    connect(this, SIGNAL(finished(int)), this, SLOT(emitDialogFinished(int)));
}

void DownloadOptionsDialog::showExternalManagerOption(bool show)
{
    ui->radioExternal->setVisible(show);
}

void DownloadOptionsDialog::showFromLine(bool show)
{
    ui->fromFrame->setVisible(show);
}

void DownloadOptionsDialog::setLastDownloadOption(const DownloadManager::DownloadOption &option)
{
    switch (option) {
    case DownloadManager::ExternalManager:
        if (!ui->radioExternal->isHidden()) {
            ui->radioExternal->setChecked(true);
            break;
        }
        // fallthrough

    case DownloadManager::OpenFile:
        ui->radioOpen->setChecked(true);
        break;

    case DownloadManager::SaveFile:
        ui->radioSave->setChecked(true);
        break;

    default:
        break;
    }
}

int DownloadOptionsDialog::exec()
{
    int status = QDialog::exec();

    if (status != 0) {
        if (ui->radioOpen->isChecked()) {
            status =  1;
        }
        else if (ui->radioSave->isChecked()) {
            status =  2;
        }
        else if (ui->radioExternal->isChecked()) {
            status = 3;
        }
    }

    return status;
}

void DownloadOptionsDialog::copyDownloadLink()
{
    QApplication::clipboard()->setText(m_downloadItem->url().toString());
    ui->copyDownloadLink->setText(tr("Download link copied."));
}

void DownloadOptionsDialog::emitDialogFinished(int status)
{
    if (status != 0) {
        if (ui->radioOpen->isChecked()) {
            status =  1;
        }
        else if (ui->radioSave->isChecked()) {
            status =  2;
        }
        else if (ui->radioExternal->isChecked()) {
            status = 3;
        }
    }

    m_signalEmited = true;
    emit dialogFinished(status);
}

DownloadOptionsDialog::~DownloadOptionsDialog()
{
    if (!m_signalEmited) {
        emit dialogFinished(-1);
    }

    delete ui;
}
