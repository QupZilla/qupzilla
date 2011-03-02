#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include "QtSql/QSqlDatabase"
#include "QSqlQuery"
#include "QDateTime"
#include "QFile"

class QupZilla;
class WebView;
class HistoryModel : public QObject
{
    Q_OBJECT
public:
    HistoryModel(QupZilla* mainClass, QObject* parent = 0);

    int addHistoryEntry(WebView* view);
    int addHistoryEntry(const QString &url, QString &title);
    bool deleteHistoryEntry(int index);
    bool deleteHistoryEntry(const QString &url, const QString &title);

    bool clearHistory();
    bool optimizeHistory();
    bool isSaving();
    void setSaving(bool state);

    void loadSettings();

private:
    bool m_isSaving;
    QupZilla* p_QupZilla;
};

#endif // HISTORYMODEL_H
