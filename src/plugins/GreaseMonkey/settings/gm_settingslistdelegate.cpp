/* ============================================================
* GreaseMonkey plugin for QupZilla
* Copyright (C) 2012-2017 David Rosca <nowrep@gmail.com>
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
#include "gm_settingslistdelegate.h"
#include "gm_script.h"

#include "iconprovider.h"

#include <QPainter>
#include <QListWidget>
#include <QApplication>

GM_SettingsListDelegate::GM_SettingsListDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , m_rowHeight(0)
    , m_padding(0)
{
    m_removePixmap = IconProvider::standardIcon(QStyle::SP_DialogCloseButton).pixmap(16);
    m_updateIcon = IconProvider::standardIcon(QStyle::SP_BrowserReload);
}

int GM_SettingsListDelegate::padding() const
{
    return m_padding;
}

void GM_SettingsListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    GM_Script *script = static_cast<GM_Script*>(index.data(Qt::UserRole + 10).value<void*>());
    if (!script)
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();
    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    // forced to LTR, see issue#967
    painter->setLayoutDirection(Qt::LeftToRight);

    // Prepare title font
    QFont titleFont = opt.font;
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);

    const QFontMetrics titleMetrics(titleFont);
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

    int leftPosition = m_padding;
    int rightPosition = opt.rect.right() - m_padding - 16; // 16 for remove button
    if (!script->downloadUrl().isEmpty())
        rightPosition -= m_padding + 16; // 16 for update button

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw checkbox
    const int checkboxSize = 18;
    const int checkboxYPos = center - (checkboxSize / 2);
    QStyleOptionViewItem opt2 = opt;
    opt2.checkState == Qt::Checked ? opt2.state |= QStyle::State_On : opt2.state |= QStyle::State_Off;
    QRect styleCheckBoxRect = style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt2, w);
    opt2.rect = QRect(leftPosition, checkboxYPos, styleCheckBoxRect.width(), styleCheckBoxRect.height());
    style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt2, painter, w);
    leftPosition = opt2.rect.right() + m_padding;

    // Draw icon
    const int iconSize = 32;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QPixmap pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    if (!pixmap.isNull()) {
        painter->drawPixmap(iconRect, pixmap);
        leftPosition = iconRect.right() + m_padding;
    } else {
        leftPosition += m_padding;
    }

    // Draw script name
    const QString name = index.data(Qt::DisplayRole).toString();
    const int leftTitleEdge = leftPosition + 2;
    const int rightTitleEdge = rightPosition - m_padding;
    const int leftPosForVersion = titleMetrics.width(name) + m_padding;
    QRect nameRect(leftTitleEdge, opt.rect.top() + m_padding, rightTitleEdge - leftTitleEdge, titleMetrics.height());
    painter->setFont(titleFont);
    style->drawItemText(painter, nameRect, Qt::AlignLeft, textPalette, true, name, colorRole);

    // Draw version
    QRect versionRect(nameRect.x() + leftPosForVersion, nameRect.y(), rightTitleEdge - leftPosForVersion, titleMetrics.height());
    QFont versionFont = titleFont;
    versionFont.setBold(false);
    painter->setFont(versionFont);
    style->drawItemText(painter, versionRect, Qt::AlignLeft, textPalette, true, script->version(), colorRole);

    // Draw description
    const int infoYPos = nameRect.bottom() + opt.fontMetrics.leading();
    QRect infoRect(nameRect.x(), infoYPos, nameRect.width(), opt.fontMetrics.height());
    const QString info = opt.fontMetrics.elidedText(script->description(), Qt::ElideRight, infoRect.width());
    painter->setFont(opt.font);
    style->drawItemText(painter, infoRect, Qt::TextSingleLine | Qt::AlignLeft, textPalette, true, info, colorRole);

    // Draw update button
    if (!script->downloadUrl().isEmpty()) {
        const int updateIconSize = 16;
        const int updateIconYPos = center - (updateIconSize / 2);
        const QPixmap updatePixmap = m_updateIcon.pixmap(16, script->isUpdating() ? QIcon::Disabled : QIcon::Normal);
        QRect updateIconRect(rightPosition, updateIconYPos, updateIconSize, updateIconSize);
        painter->drawPixmap(updateIconRect, updatePixmap);
        rightPosition += m_padding + 16;
    }

    // Draw remove button
    const int removeIconSize = 16;
    const int removeIconYPos = center - (removeIconSize / 2);
    QRect removeIconRect(rightPosition, removeIconYPos, removeIconSize, removeIconSize);
    painter->drawPixmap(removeIconRect, m_removePixmap);
}

QSize GM_SettingsListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (!m_rowHeight) {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        const QWidget* w = opt.widget;
        const QStyle* style = w ? w->style() : QApplication::style();
        const int padding = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;

        QFont titleFont = opt.font;
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 1);

        m_padding = padding > 5 ? padding : 5;

        const QFontMetrics titleMetrics(titleFont);

        m_rowHeight = 2 * m_padding + opt.fontMetrics.leading() + opt.fontMetrics.height() + titleMetrics.height();
    }

    return QSize(200, m_rowHeight);
}
