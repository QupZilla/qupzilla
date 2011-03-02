#ifndef RSSMANAGER_H
#define RSSMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QTreeWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFormLayout>
#include <QPointer>

namespace Ui {
    class RSSManager;
}

class QupZilla;
class RSSManager : public QWidget
{
    Q_OBJECT

public:
    explicit RSSManager(QupZilla* mainClass, QWidget *parent = 0);
    ~RSSManager();

    bool addRssFeed(const QString &address, const QString &title);
    void setMainWindow(QupZilla* window);

public slots:
    void refreshTable();

private slots:
    void beginToLoadSlot(const QUrl &url);
    void finished(QNetworkReply* reply);
    void loadFeed(QTreeWidgetItem* item);
    void controlLoadFeed(QTreeWidgetItem* item);
    void reloadFeed();
    void deleteFeed();
    void editFeed();
    void customContextMenuRequested(const QPoint &position);
    void loadFeedInNewTab();

private:
    QupZilla* getQupZilla();
    QList<QNetworkReply*> m_networkReplies;
    QNetworkAccessManager* m_networkManager;
    Ui::RSSManager *ui;
    QPointer<QupZilla> p_QupZilla;
};

#endif // RSSMANAGER_H
