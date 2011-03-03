/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
