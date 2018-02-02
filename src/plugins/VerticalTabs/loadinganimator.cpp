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
#include "loadinganimator.h"

#include "tabicon.h"
#include "tabmodel.h"

#include <QTimer>

class LoadingAnimation : public QObject
{
public:
    explicit LoadingAnimation(LoadingAnimator *animator)
        : QObject(animator)
        , m_animator(animator)
    {
        QTimer *timer = new QTimer(this);
        timer->setInterval(TabIcon::data()->animationInterval);
        connect(timer, &QTimer::timeout, this, [this]() {
            m_currentFrame = (m_currentFrame + 1) % TabIcon::data()->framesCount;
            m_animator->updatePixmap(this);
        });
        timer->start();
    }

    QPixmap pixmap() const
    {
        const QPixmap p = TabIcon::data()->animationPixmap;
        const int size = 16;
        const int pixmapSize = qRound(size * p.devicePixelRatioF());
        return p.copy(m_currentFrame * pixmapSize, 0, pixmapSize, pixmapSize);
    }

private:
    int m_currentFrame = 0;
    LoadingAnimator *m_animator;
};

LoadingAnimator::LoadingAnimator(QObject *parent)
    : QObject(parent)
{
}

QPixmap LoadingAnimator::pixmap(const QModelIndex &index)
{
    LoadingAnimation *animation = m_animations.value(index);
    if (!animation) {
        animation = new LoadingAnimation(this);
        m_indexes[animation] = index;
        m_animations[index] = animation;
    }
    return animation->pixmap();
}

void LoadingAnimator::updatePixmap(LoadingAnimation *animation)
{
    const QModelIndex index = m_indexes.value(animation);
    if (!index.isValid() || !index.data(TabModel::LoadingRole).toBool()) {
        animation->deleteLater();
        m_indexes.remove(animation);
        m_animations.remove(index);
    } else {
        emit updateIndex(index);
    }
}
