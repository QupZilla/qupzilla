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
#ifndef SITEINFO_H
#define SITEINFO_H

#include "qz_namespace.h"

#include <QDialog>
#include <QUrl>

namespace Ui
{
class SiteInfo;
}

class QListWidgetItem;
class QTreeWidgetItem;

class WebView;
class CertificateInfoWidget;
class ListItemDelegate;

class QT_QUPZILLA_EXPORT SiteInfo : public QDialog
{
    Q_OBJECT

public:
    explicit SiteInfo(WebView* view, QWidget* parent = 0);
    ~SiteInfo();

    static QString showCertInfo(const QString &string);

private slots:
    void databaseItemChanged(QListWidgetItem* item);
    void showImagePreview(QTreeWidgetItem* item);
    void securityDetailsClicked();

    void imagesCustomContextMenuRequested(const QPoint &p);
    void copyActionData();
    void downloadImage();

private:
    Ui::SiteInfo* ui;
    CertificateInfoWidget* m_certWidget;
    WebView* m_view;
    ListItemDelegate* m_delegate;

    QPixmap m_activePixmap;
    QUrl m_baseUrl;
};

#endif // SITEINFO_H
