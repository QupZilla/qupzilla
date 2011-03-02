#include "downloadmanager.h"
#include "ui_downloadmanager.h"
#include "qupzilla.h"
#include "downloadoptionsdialog.h"
#include "downloaditem.h"
#include "ecwin7.h"

DownloadManager::DownloadManager(QWidget *parent) :
    QWidget(parent)
    ,ui(new Ui::DownloadManager)
    ,m_isClosing(false)
{
    ui->setupUi(this);
    ui->clearButton->setIcon(QIcon::fromTheme("edit-clear"));
    //CENTER on screen
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );

    m_iconProvider = new QFileIconProvider();
    m_networkManager = new QNetworkAccessManager();

    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    m_downloadPath = settings.value("defaultDownloadPath", QDir::homePath()).toString();
    m_lastDownloadPath = settings.value("lastDownloadPath","").toString();
    settings.endGroup();

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

#ifdef Q_WS_WIN
    win7.init(this->winId());
#endif
}

#ifdef Q_WS_WIN
bool DownloadManager::winEvent(MSG *message, long *result)
{
    return win7.winEvent(message, result);
}
#endif

void DownloadManager::timerEvent(QTimerEvent *event)
{
    QList<QTime> remTimes;
    QList<int> progresses;
    QList<double> speeds;

    if (event->timerId() == m_timer.timerId()) {
        if (!ui->list->count()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
            return;
        }
        for (int i = 0; i < ui->list->count(); i++) {
            DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
            if (!downItem || (downItem && downItem->isCancelled()))
                continue;
            if (!downItem->isDownloading()) {
                progresses.append(100);
                continue;
            } else
                progresses.append(downItem->progress());
            remTimes.append(downItem->remainingTime());
            speeds.append(downItem->currentSpeed());
        }
        if (remTimes.isEmpty()) {
            ui->speedLabel->clear();
            setWindowTitle(tr("Download Manager"));
            return;
        }

        QTime remaining;
        foreach (QTime time, remTimes) {
            if (time > remaining)
                remaining = time;
        }

        int progress = 0;
        foreach (int prog, progresses)
            progress+=prog;
        progress = progress / progresses.count();

        double speed = 0.00;
        foreach (double spee, speeds)
            speed+=spee;

        ui->speedLabel->setText(tr("%1% of %2 files (%3) %4 remaining").arg(QString::number(progress),QString::number(progresses.count()),
                                                                            DownloadItem::currentSpeedToString(speed),
                                                                            DownloadItem::remaingTimeToString(remaining)));
        setWindowTitle(QString::number(progress) + tr("% - Download Manager"));
#ifdef Q_WS_WIN
        win7.setProgressValue(progress, 100);
        win7.setProgressState(win7.Normal);
#endif
    } else
        QWidget::timerEvent(event);
}

void DownloadManager::clearList()
{
    QList<DownloadItem*> items;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem)
            continue;
        if (downItem->isDownloading())
            continue;
        items.append(downItem);
    }
    qDeleteAll(items);
}

void DownloadManager::download(const QNetworkRequest &request)
{
    handleUnsupportedContent(m_networkManager->get(request));
}

void DownloadManager::handleUnsupportedContent(QNetworkReply *reply)
{
//    DownloadOptionsDialog* dialog = new DownloadOptionsDialog();
//    dialog->show();
//    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QString path;
    QString fileName;

    QString userFileName;

    QString _fileName = getFileName(reply);

    if (m_downloadPath.isEmpty())
        userFileName = QFileDialog::getSaveFileName(MainApplication::getInstance()->getWindow(), tr("Save file as..."),m_lastDownloadPath+_fileName);
    else
        userFileName = m_downloadPath+_fileName;

    if (userFileName.isEmpty()) {
        reply->abort();
        return;
    }

    int pos = userFileName.lastIndexOf("/");
    if (pos!=-1) {
        int size = userFileName.size();
        path = userFileName.left(pos+1);
        fileName = userFileName.right(size-pos-1);
    }

    m_lastDownloadPath = path;
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("DownloadManager");
    settings.setValue("lastDownloadPath",m_lastDownloadPath);
    settings.endGroup();

    QFileInfo info(reply->url().toString());
    QTemporaryFile tempFile("XXXXXX."+info.suffix());
    tempFile.open();
    QFileInfo tempInfo(tempFile.fileName());
    QPixmap fileIcon = m_iconProvider->icon(tempInfo).pixmap(30,30);

    QListWidgetItem* item = new QListWidgetItem(ui->list);
    DownloadItem* downItem = new DownloadItem(item, reply, path, fileName, fileIcon);
    connect(downItem, SIGNAL(deleteItem(DownloadItem*)), this, SLOT(deleteItem(DownloadItem*)));
    ui->list->setItemWidget(item, downItem);
    item->setSizeHint(downItem->sizeHint());
    show();
    activateWindow();
}

void DownloadManager::deleteItem(DownloadItem *item)
{
    if (item && !item->isDownloading())
        delete item;
}

QString DownloadManager::getFileName(QNetworkReply *reply)
{
    QString path;
    if (reply->hasRawHeader("Content-Disposition")) {
        QString value = reply->rawHeader("Content-Disposition");
        int pos = value.indexOf("filename=");
        if (pos!=-1) {
            QString name = value.mid(pos + 9);
            if (name.startsWith('"') && name.endsWith('"'))
                name = name.mid(1, name.size() - 2);
            path = name;
           }
       }
    if (path.isEmpty())
        path = reply->url().path();

    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();

    if (baseName.isEmpty()) {
        baseName = tr("NoNameDownload");
    }

    if (!endName.isEmpty())
        endName="."+endName;

    return baseName+endName;
}

bool DownloadManager::canClose()
{
    if (m_isClosing)
        return true;

    bool isDownloading = false;
    for (int i = 0; i < ui->list->count(); i++) {
        DownloadItem* downItem = qobject_cast<DownloadItem*>(ui->list->itemWidget(ui->list->item(i)));
        if (!downItem)
            continue;
        if (downItem->isDownloading()) {
            isDownloading = true;
            break;
        }
    }

    return !isDownloading;
}

void DownloadManager::closeEvent(QCloseEvent *e)
{
    if (!MainApplication::getInstance()->getWindow()) { // No main windows -> we are going to quit
        if (!canClose()){
            QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Warning"),
                             tr("Are you sure to quit? All uncompleted downloads will be cancelled!"), QMessageBox::Yes | QMessageBox::No);
            if (button != QMessageBox::Yes) {
                e->ignore();
                return;
            }
            m_isClosing = true;
        }
        MainApplication::getInstance()->quitApplication();
    }
    e->accept();
}

DownloadManager::~DownloadManager()
{
    delete ui;
    delete m_networkManager;
    delete m_iconProvider;
}
