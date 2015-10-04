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
#ifndef DOWNLOADOPTIONSDIALOG_H
#define DOWNLOADOPTIONSDIALOG_H

#include <QDialog>
#include <QUrl>

#include "qzcommon.h"
#include "downloadmanager.h"

namespace Ui
{
class DownloadOptionsDialog;
}

class QUPZILLA_EXPORT DownloadOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadOptionsDialog(const QString &fileName, const QUrl &url, QWidget* parent = 0);
    ~DownloadOptionsDialog();

    void showExternalManagerOption(bool show);
    void showFromLine(bool show);

    void setLastDownloadOption(const DownloadManager::DownloadOption &option);

    int exec();

private slots:
    void copyDownloadLink();
    void emitDialogFinished(int status);

signals:
    void dialogFinished(int);

private:
    Ui::DownloadOptionsDialog* ui;

    QUrl m_url;
    bool m_signalEmited;
};

#endif // DOWNLOADOPTIONSDIALOG_H
