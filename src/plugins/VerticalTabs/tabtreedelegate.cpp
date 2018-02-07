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
#include "tabtreedelegate.h"
#include "tabtreeview.h"
#include "loadinganimator.h"

#include "tabmodel.h"
#include "tabicon.h"

#include <QTabBar>
#include <QPainter>
#include <QPushButton>
#include <QApplication>

// TabTreeCloseButton
TabTreeCloseButton::TabTreeCloseButton(QWidget *parent)
    : QAbstractButton(parent)
{
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this);
    resize(width, height);
}

int TabTreeCloseButton::showOnNormal() const
{
    return m_showOnNormal;
}

void TabTreeCloseButton::setShowOnNormal(int show)
{
    m_showOnNormal = show;
}

int TabTreeCloseButton::showOnHovered() const
{
    return m_showOnHovered;
}

void TabTreeCloseButton::setShowOnHovered(int show)
{
    m_showOnHovered = show;
}

int TabTreeCloseButton::showOnSelected() const
{
    return m_showOnSelected;
}

void TabTreeCloseButton::setShowOnSelected(int show)
{
    m_showOnSelected = show;
}

bool TabTreeCloseButton::isVisible(bool hovered, bool selected) const
{
    if (hovered && selected) {
        return m_showOnHovered || m_showOnSelected;
    } else if (selected) {
        return m_showOnSelected;
    } else if (hovered) {
        return m_showOnHovered;
    } else {
        return m_showOnNormal;
    }
}

void TabTreeCloseButton::paintEvent(QPaintEvent *)
{
}

// TabTreeDelegate
TabTreeDelegate::TabTreeDelegate(TabTreeView *view)
    : QStyledItemDelegate()
    , m_view(view)
{
    m_padding = qMax(5, m_view->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1);
    m_indentation = 15;

    m_loadingAnimator = new LoadingAnimator(this);
    connect(m_loadingAnimator, &LoadingAnimator::updateIndex, m_view, &TabTreeView::updateIndex);

    // Needed to make it stylable the same way as real tabbar close button
    QTabBar *tabBar = new QTabBar(m_view);
    tabBar->setObjectName(QSL("tabtree_tabbar"));
    tabBar->lower();

    m_closeButton = new TabTreeCloseButton(tabBar);
    m_closeButton->lower();
}

static int indexDepth(QModelIndex index)
{
    int i = 0;
    while (index.parent().isValid()) {
        index = index.parent();
        i++;
    }
    return i;
}

QRect TabTreeDelegate::expandButtonRect(const QModelIndex &index) const
{
    const QRect rect = m_view->visualRect(index);
    const int depth = indexDepth(index);
    return QRect(m_indentation * depth, rect.y(), m_indentation, rect.height());
}

QRect TabTreeDelegate::audioButtonRect(const QModelIndex &index) const
{
    if (!index.data(TabModel::AudioPlayingRole).toBool() && !index.data(TabModel::AudioMutedRole).toBool()) {
        return QRect();
    }
    const QRect rect = m_view->visualRect(index);
    const int center = rect.height() / 2 + rect.top();
    const int rightPosition = rect.right() - m_padding * 2 - 16;
    return QRect(rightPosition - 16, center - 16 / 2, 16, 16);
}

QRect TabTreeDelegate::closeButtonRect(const QModelIndex &index) const
{
    const QRect rect = m_view->visualRect(index);
    const int center = rect.height() / 2 + rect.top();
    QSize size = m_closeButton->size();
    size.setHeight(qMin(rect.height() - m_padding, size.height()));
    return QRect(QPoint(rect.right() - m_padding - size.width(), center - size.height() / 2), size);
}

void TabTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QWidget *w = option.widget;
    const QStyle *style = w ? w->style() : m_view->style();

    const bool expanded = m_view->isExpanded(index);
    const bool children = m_view->model()->rowCount(index) > 0;
    const int depth = indexDepth(index);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    m_view->adjustStyleOption(&opt);

    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    int leftPosition = opt.rect.left() + m_indentation + m_indentation * depth + m_padding;
    int rightPosition = opt.rect.right() - m_padding * 2 - m_closeButton->size().width();

    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;

    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

#ifdef Q_OS_WIN
    opt.palette.setColor(QPalette::All, QPalette::HighlightedText, opt.palette.color(QPalette::Active, QPalette::Text));
    opt.palette.setColor(QPalette::All, QPalette::Highlight, opt.palette.base().color().darker(108));
#endif

    QPalette textPalette = opt.palette;
    textPalette.setCurrentColorGroup(cg);

    const bool hovered = opt.state.testFlag(QStyle::State_MouseOver);
    const bool selected = opt.state.testFlag(QStyle::State_Selected);

    // Draw background
    if (m_view->backgroundIndentation()) {
        opt.rect.moveLeft(m_indentation * depth);
        opt.rect.setWidth(opt.rect.width() - m_indentation * depth);
    }
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw expand button
    if (children) {
        QStyleOptionViewItem o = opt;
        o.state &= ~QStyle::State_MouseOver;
        o.rect.moveLeft(m_indentation * depth);
        o.rect.setWidth(m_indentation);
        style->drawPrimitive(expanded ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowRight, &o, painter, w);
    }

    // Draw icon
    const int iconSize = 16;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QPixmap pixmap;
    if (index.data(TabModel::LoadingRole).toBool()) {
        pixmap = m_loadingAnimator->pixmap(index);
    } else {
        pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    }
    painter->drawPixmap(iconRect, pixmap);
    leftPosition += iconRect.width() + m_padding;

    // Draw close button
    if (m_closeButton->isVisible(hovered, selected)) {
        QStyleOptionButton o;
        o.init(m_closeButton);

        const bool hovered = closeButtonRect(index).contains(m_view->viewport()->mapFromGlobal(QCursor::pos()));
        const bool pressed = hovered && QApplication::mouseButtons() == Qt::LeftButton;

        QSize closeSize = QSize(o.rect.size().width(), qMin(height - m_padding, o.rect.size().height()));
        QPoint pos(opt.rect.right() - m_padding - closeSize.width(), center - closeSize.height() / 2);
        o.rect = QRect(pos, closeSize);
        o.state |= QStyle::State_AutoRaise | QStyle::State_Enabled | QStyle::State_Selected;
        o.state.setFlag(QStyle::State_Raised, hovered && !pressed);
        o.state.setFlag(QStyle::State_Sunken, pressed);
        o.state.setFlag(QStyle::State_MouseOver, hovered);
        style->drawPrimitive(QStyle::PE_IndicatorTabClose, &o, painter, m_closeButton);
    }

    // Draw audio icon
    const bool audioMuted = index.data(TabModel::AudioMutedRole).toBool();
    const bool audioPlaying = index.data(TabModel::AudioPlayingRole).toBool();
    if (audioMuted || audioPlaying) {
        QSize audioSize(16, 16);
        QPoint pos(rightPosition - audioSize.width(), center - audioSize.height() / 2);
        QRect audioRect(pos, audioSize);
        painter->drawPixmap(audioRect, audioMuted ? TabIcon::data()->audioMutedPixmap : TabIcon::data()->audioPlayingPixmap);
        rightPosition -= audioSize.width() + m_padding;
    }

    // Draw title
    QRect titleRect(leftPosition, center - opt.fontMetrics.height() / 2, opt.rect.width(), opt.fontMetrics.height());
    titleRect.setRight(rightPosition - m_padding);
    QString title = opt.fontMetrics.elidedText(index.data().toString(), Qt::ElideRight, titleRect.width());
    style->drawItemText(painter, titleRect, Qt::AlignLeft, textPalette, true, title, colorRole);
}

QSize TabTreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    return QSize(200, m_padding * 2 + opt.fontMetrics.height());
}
