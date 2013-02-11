/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#ifndef LOCATIONCOMPLETER_H
#define LOCATIONCOMPLETER_H

#include <QObject>

#include "qz_namespace.h"

class QModelIndex;

class LocationCompleterModel;
class LocationCompleterView;
class LocationBar;

class QT_QUPZILLA_EXPORT LocationCompleter : public QObject
{
    Q_OBJECT
public:
    explicit LocationCompleter(QObject* parent = 0);

    void setLocationBar(LocationBar* locationBar);

    bool showingMostVisited() const;
    bool isPopupVisible() const;
    void closePopup();

signals:
    void showCompletion(const QString &);
    void completionActivated();

public slots:
    void complete(const QString &string);
    void showMostVisited();

private slots:
    void currentChanged(const QModelIndex &index);
    void popupClosed();

private:
    void showPopup();
    void adjustPopupSize();

    LocationBar* m_locationBar;
    QString m_originalText;
    bool m_ignoreCurrentChangedSignal;
    bool m_showingMostVisited;

    static LocationCompleterView* s_view;
    static LocationCompleterModel* s_model;
};

#endif // LOCATIONCOMPLETER_H
