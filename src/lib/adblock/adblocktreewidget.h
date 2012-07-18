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
#ifndef ADBLOCKTREEWIDGET_H
#define ADBLOCKTREEWIDGET_H

#include <QTreeWidget>

#include "qz_namespace.h"
#include "treewidget.h"

class AdBlockSubscription;
class AdBlockRule;

class QT_QUPZILLA_EXPORT AdBlockTreeWidget : public TreeWidget
{
    Q_OBJECT
public:
    explicit AdBlockTreeWidget(AdBlockSubscription* subscription, QWidget* parent = 0);

    AdBlockSubscription* subscription() const;

    void showRule(const AdBlockRule* rule);
    void refresh();

public slots:
    void addRule();
    void removeRule();

private slots:
    void contextMenuRequested(const QPoint &pos);
    void itemChanged(QTreeWidgetItem* item);
    void copyFilter();

    void subscriptionUpdated();

private:
    void adjustItemFeatures(QTreeWidgetItem* item, const AdBlockRule &rule);
    void keyPressEvent(QKeyEvent* event);

    AdBlockSubscription* m_subscription;
    QTreeWidgetItem* m_topItem;

    QString m_ruleToBeSelected;
    bool m_itemChangingBlock;
};

#endif // ADBLOCKTREEWIDGET_H
