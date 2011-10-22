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

namespace Ui {
    class BookmarksImportDialog;
}

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
    void iconChanged();
    void loadFinished();

private:
    enum Browser { Firefox = 0, Chrome = 1, Opera = 2, IE = 3};

    void setupBrowser(Browser browser);
    bool exportedOK();
    void startFetchingIcons();
    void addExportedBookmarks();

    Ui::BookmarksImportDialog *ui;

    int m_currentPage;
    Browser m_browser;
    QString m_browserName;
    QString m_browserFileText;
    QString m_browserFileText2;
    QString m_standardDir;

    QPixmap m_browserPixmap;
    QString m_browserBookmarkFile;

    QList<BookmarksModel::Bookmark> m_exportedBookmarks;

    QList<QPair<QWebFrame*, QUrl> > m_webViews;
};

#endif // BOOKMARKSIMPORTDIALOG_H
