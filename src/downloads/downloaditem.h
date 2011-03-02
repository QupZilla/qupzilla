#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QFileInfo>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>

namespace Ui {
    class DownloadItem;
}

class DownloadItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadItem(QListWidgetItem* item, QNetworkReply* reply ,QString path, QString fileName, QPixmap fileIcon, QWidget *parent = 0);
    bool isDownloading() { return m_downloading; }
    bool isCancelled();
    QTime remainingTime() { return m_remTime; }
    double currentSpeed() { return m_currSpeed; }
    int progress();
    ~DownloadItem();

    static QString remaingTimeToString(QTime time);
    static QString currentSpeedToString(double speed);
    static QString fileSizeToString(int size);

signals:
    void deleteItem(DownloadItem*);

private slots:
    void finished();
    void downloadProgress(qint64 received, qint64 total);
    void stop();
    void openFile();
    void openFolder();
    void readyRead();
    void error(QNetworkReply::NetworkError);
    void updateDownload();
    void customContextMenuRequested(QPoint pos);
    void clear();

private:
    void timerEvent(QTimerEvent *event);
    void updateDownloadInfo(double currSpeed, qint64 received, qint64 total);
    void mouseDoubleClickEvent(QMouseEvent *e);
    Ui::DownloadItem *ui;

    QListWidgetItem* m_item;
    QNetworkReply* m_reply;
    QString m_path;
    QString m_fileName;
    QTime m_downTimer;
    QTime m_remTime;
    QBasicTimer m_timer;
    QFile m_outputFile;

    bool m_downloading;
    double m_currSpeed;
    qint64 m_received;
    qint64 m_total;
};

#endif // DOWNLOADITEM_H
