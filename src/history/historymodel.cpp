#include "historymodel.h"
#include "webview.h"
#include "qupzilla.h"

HistoryModel::HistoryModel(QupZilla *mainClass, QObject *parent)
    : QObject(parent)
    ,m_isSaving(true)
    ,p_QupZilla(mainClass)
{
    loadSettings();
}

void HistoryModel::loadSettings()
{
    QSettings settings(MainApplication::getInstance()->getActiveProfil()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Web-Browser-Settings");
    m_isSaving = settings.value("allowHistory",true).toBool();
}

int HistoryModel::addHistoryEntry(const QString &url, QString &title)
{
    if (!m_isSaving)
        return -2;
    if (url.contains("file://") || title.contains(tr("Failed loading page")) || url.isEmpty() || url.contains("about:blank") )
        return -1;

    QSqlQuery query;
    query.prepare("SELECT id FROM history WHERE url=?");
    query.bindValue(0, url);
    query.exec();
    if (!query.next()) {
        QDateTime now = QDateTime::currentDateTime();
        if (title == "")
            title=tr("No Named Page");
        query.prepare("INSERT INTO history (count, date, url, title) VALUES (1,?,?,?)");
        query.bindValue(0, now.toMSecsSinceEpoch());
        query.bindValue(1, url);
        query.bindValue(2, title);
        query.exec();
    }else{
        QDateTime now = QDateTime::currentDateTime();
        if (title == "")
            title=tr("No Named Page");
        query.prepare("UPDATE history SET count = count + 1, date=? WHERE url=?");
        query.bindValue(0, now.toMSecsSinceEpoch());
        query.bindValue(1, url);
        query.exec();
    }
    return query.lastInsertId().toInt();
}

int HistoryModel::addHistoryEntry(WebView *view)
{
    if (!m_isSaving)
        return -2;
    QString url = view->url().toString();
    QString title = view->title();
    return addHistoryEntry(url, title);
}

bool HistoryModel::deleteHistoryEntry(int index)
{
    QSqlQuery query;
    query.prepare("DELETE FROM history WHERE id=?");
    query.bindValue(0, index);
    if (query.exec())
        return true;
    else return false;
}

bool HistoryModel::deleteHistoryEntry(const QString &url, const QString &title)
{
    QSqlQuery query;
    query.prepare("DELETE FROM history WHERE url=? AND title=?");
    query.bindValue(0, url);
    query.bindValue(1, title);
    if (query.exec())
        return true;
    else return false;
}

bool HistoryModel::optimizeHistory()
{
    QSqlQuery query;
    return query.exec("VACUUM");
}

bool HistoryModel::clearHistory()
{
    QSqlQuery query;
    return query.exec("DELETE FROM history");
}

void HistoryModel::setSaving(bool state)
{
    m_isSaving = state;
}

bool HistoryModel::isSaving()
{
    return m_isSaving;
}
