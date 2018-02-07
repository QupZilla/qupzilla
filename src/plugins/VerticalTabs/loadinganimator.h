/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include <QHash>
#include <QObject>
#include <QPersistentModelIndex>

class LoadingAnimation;

class LoadingAnimator : public QObject
{
    Q_OBJECT

public:
    explicit LoadingAnimator(QObject *parent = nullptr);

    QPixmap pixmap(const QModelIndex &index);

signals:
    void updateIndex(const QModelIndex &index);

private:
    void updatePixmap(LoadingAnimation *animation);

    QHash<LoadingAnimation*, QPersistentModelIndex> m_indexes;
    QHash<QPersistentModelIndex, LoadingAnimation*> m_animations;

    friend class LoadingAnimation;
};
