#ifndef WEBHISTORYINTERFACE_H
#define WEBHISTORYINTERFACE_H

#include <QWebHistoryInterface>

class WebHistoryInterface : public QWebHistoryInterface
{
    Q_OBJECT
public:
    explicit WebHistoryInterface(QObject *parent = 0);

    void addHistoryEntry(const QString &url);
    bool historyContains(const QString &url) const;

signals:

public slots:

private:
    QList<QString> m_clickedLinks;

};

#endif // WEBHISTORYINTERFACE_H
