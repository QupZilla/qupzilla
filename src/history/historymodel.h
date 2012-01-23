/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QFile>
#include <QIcon>
#include <QUrl>
#include <QThread>

class QupZilla;
class WebView;
class QIcon;
class HistoryModel : public QObject
{
    Q_OBJECT
public:
    HistoryModel(QupZilla* mainClass);

    struct HistoryEntry {
        int id;
        int count;
        QDateTime date;
        QUrl url;
        QString title;
    };

    static QString titleCaseLocalizedMonth(int month);

    void addHistoryEntry(WebView* view);
    void addHistoryEntry(const QUrl &url, QString title);

    void deleteHistoryEntry(int index);
    void deleteHistoryEntry(const QString &url, const QString &title);

    bool urlIsStored(const QString &url);

    QList<HistoryEntry> mostVisited(int count);

    bool clearHistory();
    bool optimizeHistory();
    bool isSaving();
    void setSaving(bool state);

    void loadSettings();

private slots:
    void slotAddHistoryEntry(const QUrl &url, QString title);
    void slotDeleteHistoryEntry(int index);

signals:
    void historyEntryAdded(HistoryEntry entry);
    void historyEntryDeleted(HistoryEntry entry);
    void historyEntryEdited(HistoryEntry before, HistoryEntry after);
    //WARNING: Incomplete HistoryEntry structs are passed to historyEntryEdited!
    void historyClear();

    void signalAddHistoryEntry(QUrl url, QString title);
    void signalDeleteHistoryEntry(int index);

private:
    bool m_isSaving;
    QupZilla* p_QupZilla;
};

typedef HistoryModel::HistoryEntry HistoryEntry;

#endif // HISTORYMODEL_H
