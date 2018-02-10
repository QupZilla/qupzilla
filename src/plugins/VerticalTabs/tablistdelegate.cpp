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
#include "tablistdelegate.h"
#include "tablistview.h"
#include "loadinganimator.h"

#include "tabmodel.h"
#include "tabicon.h"

#include <QPainter>

TabListDelegate::TabListDelegate(TabListView *view)
    : QStyledItemDelegate()
    , m_view(view)
{
    m_padding = qMax(5, m_view->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1);

    m_loadingAnimator = new LoadingAnimator(this);
    connect(m_loadingAnimator, &LoadingAnimator::updateIndex, m_view, &TabListView::updateIndex);
}

QRect TabListDelegate::audioButtonRect(const QModelIndex &index) const
{
    if (!index.data(TabModel::AudioPlayingRole).toBool() && !index.data(TabModel::AudioMutedRole).toBool()) {
        return QRect();
    }
    const QRect rect = m_view->visualRect(index);
    const int center = rect.height() / 2 + rect.top();
    return QRect(rect.right() - 16, center - 16 / 2, 16, 16);
}

void TabListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QWidget *w = option.widget;
    const QStyle *style = w ? w->style() : m_view->style();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    m_view->adjustStyleOption(&opt);

    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    painter->setRenderHint(QPainter::Antialiasing);

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw icon
    const int iconSize = 16;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(opt.rect.left() + (opt.rect.width() - iconSize) / 2, iconYPos, iconSize, iconSize);
    QPixmap pixmap;
    if (index.data(TabModel::LoadingRole).toBool()) {
        pixmap = m_loadingAnimator->pixmap(index);
    } else {
        pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    }
    painter->drawPixmap(iconRect, pixmap);

    // Draw audio icon
    const bool audioMuted = index.data(TabModel::AudioMutedRole).toBool();
    const bool audioPlaying = index.data(TabModel::AudioPlayingRole).toBool();
    if (audioMuted || audioPlaying) {
        QSize audioSize(16, 16);
        QPoint pos(opt.rect.right() - audioSize.width(), center - audioSize.height() / 2);
        QRect audioRect(pos, audioSize);

        QColor c = opt.palette.color(QPalette::Window);
        c.setAlpha(180);
        painter->setPen(c);
        painter->setBrush(c);
        painter->drawEllipse(audioRect);

        painter->drawPixmap(audioRect, audioMuted ? TabIcon::data()->audioMutedPixmap : TabIcon::data()->audioPlayingPixmap);
    }

    // Draw background activity indicator
    const bool backgroundActivity = index.data(TabModel::BackgroundActivityRole).toBool();
    if (backgroundActivity) {
        QSize activitySize(7, 7);
        QPoint pos(iconRect.center().x() - activitySize.width() / 2 + 1, iconRect.bottom() - 2);
        QRect activityRect(pos, activitySize);

        QColor c1 = opt.palette.color(QPalette::Window);
        c1.setAlpha(180);
        painter->setPen(Qt::transparent);
        painter->setBrush(c1);
        painter->drawEllipse(activityRect);

        const QRect r2 = activityRect.adjusted(1, 1, -1, -1);
        painter->setPen(Qt::transparent);
        painter->setBrush(opt.palette.color(QPalette::Text));
        painter->drawEllipse(r2);
    }
}

QSize TabListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    return QSize(m_padding * 4 + 16, m_padding * 2 + opt.fontMetrics.height());
}
