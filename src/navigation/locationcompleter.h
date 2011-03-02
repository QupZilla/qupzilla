#ifndef LOCATIONCOMPLETER_H
#define LOCATIONCOMPLETER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QCompleter>
#include <QDebug>
#include <QStringList>
#include <QSqlQuery>
#include <QTreeView>
#include <QStandardItemModel>
#include <QTimer>
#include <QHeaderView>
#include <QUrl>

class LocationCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit LocationCompleter(QObject *parent = 0);

    //virtual QString pathFromIndex(const QModelIndex &index) const;
    virtual QStringList splitPath(const QString &path) const;

signals:

public slots:
    void loadInitialHistory();
    void refreshCompleter(QString string);

};

#endif // LOCATIONCOMPLETER_H
