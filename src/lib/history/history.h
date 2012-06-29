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
#ifndef HISTORY_H
#define HISTORY_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QUrl>

#include "qz_namespace.h"

class QIcon;

class QupZilla;
class WebView;
class HistoryModel;

class QT_QUPZILLA_EXPORT History : public QObject
{
    Q_OBJECT
public:
    History(QObject* parent);

    struct HistoryEntry {
        int id;
        int count;
        QDateTime date;
        QUrl url;
        QString urlString;
        QString title;
    };

    static QString titleCaseLocalizedMonth(int month);

    HistoryModel* model();

    void addHistoryEntry(WebView* view);
    void addHistoryEntry(const QUrl &url, QString title);

    void deleteHistoryEntry(int index);
    void deleteHistoryEntry(const QList<int> &list);
    void deleteHistoryEntry(const QString &url, const QString &title);

    QList<int> indexesFromTimeRange(qint64 start, qint64 end);

    bool urlIsStored(const QString &url);

    QList<HistoryEntry> mostVisited(int count);

    bool clearHistory();
    bool optimizeHistory();
    bool isSaving();
    void setSaving(bool state);

    void loadSettings();

signals:
    void historyEntryAdded(const HistoryEntry &entry);
    void historyEntryDeleted(const HistoryEntry &entry);
    void historyEntryEdited(const HistoryEntry &before, const HistoryEntry &after);

    void resetHistory();

private:
    bool m_isSaving;

    QupZilla* p_QupZilla;
    HistoryModel* m_model;
};

typedef History::HistoryEntry HistoryEntry;

#endif // HISTORY_H
