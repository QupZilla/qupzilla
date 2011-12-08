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
#ifndef BOOKMARKSIMPORTDIALOG_H
#define BOOKMARKSIMPORTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QPair>
#include <QWebPage>
#include <QWebFrame>
#include <QWebSettings>

#include "bookmarksmodel.h"

namespace Ui
{
class BookmarksImportDialog;
}

class IconFetcher;
class BookmarksImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksImportDialog(QWidget* parent = 0);
    ~BookmarksImportDialog();

private slots:
    void nextPage();
    void setFile();

    void stopDownloading();
    void iconFetched(const QIcon &icon);
    void loadFinished();

private:
    enum Browser { Firefox = 0, Chrome = 1, Opera = 2, Html = 3, IE = 4};

    void setupBrowser(Browser browser);
    bool exportedOK();
    void startFetchingIcons();
    void addExportedBookmarks();

    Ui::BookmarksImportDialog* ui;

    int m_currentPage;
    Browser m_browser;
    QString m_browserName;
    QString m_browserFileText;
    QString m_browserFileText2;
    QString m_standardDir;

    QPixmap m_browserPixmap;
    QString m_browserBookmarkFile;

    QList<BookmarksModel::Bookmark> m_exportedBookmarks;

    QList<QPair<IconFetcher*, QUrl> > m_fetchers;
};

#endif // BOOKMARKSIMPORTDIALOG_H
