#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QDialog>
#include <QLabel>
#include <QFileIconProvider>
#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QDateTime>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QNetworkReply>

namespace Ui {
    class DownloadManager;
}

class DownloadItem;
class EcWin7;
class DownloadManager : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadManager(QWidget *parent = 0);
    ~DownloadManager();

    void download(const QNetworkRequest &request);
    void handleUnsupportedContent(QNetworkReply* reply);
    bool canClose();

    void show() { m_timer.start(1000*2, this); QWidget::show(); }

#ifdef Q_WS_WIN
protected:
    virtual bool winEvent(MSG *message, long *result);
#endif

private slots:
    void clearList();
    void deleteItem(DownloadItem* item);

private:
#ifdef Q_WS_WIN
    EcWin7 win7;
#endif
    void timerEvent(QTimerEvent *event);
    QString getFileName(QNetworkReply* reply);
    void closeEvent(QCloseEvent *e);

    Ui::DownloadManager *ui;
    QNetworkAccessManager* m_networkManager;
    QFileIconProvider* m_iconProvider;

    QString m_lastDownloadPath;
    QString m_downloadPath;
    QBasicTimer m_timer;

    bool m_isClosing;
};

#endif // DOWNLOADMANAGER_H
