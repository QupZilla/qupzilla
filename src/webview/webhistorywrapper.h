#ifndef WEBHISTORYWRAPPER_H
#define WEBHISTORYWRAPPER_H

#include <QObject>
#include <QWebHistory>


class WebHistoryWrapper : public QObject
{
    Q_OBJECT
public:
    explicit WebHistoryWrapper(QObject *parent = 0);

    static QList<QWebHistoryItem> forwardItems(int maxItems, QWebHistory* history);
    static QList<QWebHistoryItem> backItems(int maxItems, QWebHistory* history);

    static bool canGoForward(QWebHistory* history);
    static bool canGoBack(QWebHistory* history);

    static void goBack(QWebHistory* history);
    static void goForward(QWebHistory* history);


signals:

};

#endif // WEBHISTORYWRAPPER_H
