#ifndef COOKIEMANAGER_H
#define COOKIEMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QTimer>
#include <QNetworkCookie>
#include <QTreeWidgetItem>

namespace Ui {
    class CookieManager;
}

class QupZilla;
class CookieManager : public QWidget
{
    Q_OBJECT

public:
    explicit CookieManager(QWidget *parent = 0);
    ~CookieManager();

public slots:
    void refreshTable(bool refreshCookieJar = true);

private slots:
    void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* parent);
    void removeCookie();
    void removeAll();
    void search();

private:
    Ui::CookieManager *ui;

    QList<QNetworkCookie> m_cookies;
};

#endif // COOKIEMANAGER_H
