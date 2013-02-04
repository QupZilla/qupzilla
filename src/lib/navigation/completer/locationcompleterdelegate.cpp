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
#include "locationcompleterdelegate.h"
#include "locationcompleterview.h"
#include "locationcompletermodel.h"
#include "iconprovider.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

LocationCompleterDelegate::LocationCompleterDelegate(LocationCompleterView* parent)
    : QStyledItemDelegate(parent)
    , m_rowHeight(0)
    , m_padding(0)
    , m_drawSwitchToTab(true)
    , m_view(parent)
{
}

void LocationCompleterDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();

    const int height = opt.rect.height();
    const int center = height / 2 + opt.rect.top();

    // Prepare title font
    QFont titleFont = opt.font;
    titleFont.setPointSize(titleFont.pointSize() + 1);

    const QFontMetrics titleMetrics(titleFont);

    int leftPosition = m_padding * 2;
    int rightPosition = opt.rect.right() - m_padding;

    opt.state &= ~QStyle::State_MouseOver;

    if (m_view->hoveredIndex() == index) {
        opt.state |= QStyle::State_Selected;
    }
    else {
        opt.state &= ~QStyle::State_Selected;
    }

#ifdef Q_OS_WIN
    const QPalette::ColorRole colorRole = QPalette::Text;
    const QPalette::ColorRole colorLinkRole = QPalette::Link;
#else
    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;
    const QPalette::ColorRole colorLinkRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Link;
#endif

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw icon
    const int iconSize = 16;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QPixmap pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    painter->drawPixmap(iconRect, pixmap);
    leftPosition = iconRect.right() + m_padding * 2;

    // Draw star to bookmark items
    int starPixmapWidth = 0;
    if (index.data(LocationCompleterModel::BookmarkRole).toBool()) {
        const QPixmap starPixmap = qIconProvider->bookmarkIcon();
        QSize starSize = starPixmap.size();
        //new
        starPixmapWidth = starSize.width();
        QPoint pos(rightPosition - starPixmapWidth, opt.rect.top() + m_padding);
        QRect starRect(pos, starSize);
        painter->drawPixmap(starRect, starPixmap);
    }

    const QString &searchText = index.data(LocationCompleterModel::SearchStringRole).toString();

    // Draw title
    const int leftTitleEdge = leftPosition + 2;
    // RTL Support: remove conflicting of right-aligned text and starpixmap!
    const int rightTitleEdge = rightPosition - m_padding - starPixmapWidth;
    QRect titleRect(leftTitleEdge, opt.rect.top() + m_padding, rightTitleEdge - leftTitleEdge, titleMetrics.height());
    QString title(titleMetrics.elidedText(index.data(LocationCompleterModel::TitleRole).toString(), Qt::ElideRight, titleRect.width()));
    painter->setFont(titleFont);

    drawHighlightedTextLine(titleRect, title, searchText, painter, style, opt, colorRole);

    // Draw link
    const int infoYPos = titleRect.bottom() + opt.fontMetrics.leading() + 2;
    QRect linkRect(titleRect.x(), infoYPos, titleRect.width(), opt.fontMetrics.height());
    QString link(opt.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideRight, linkRect.width()));
    painter->setFont(opt.font);
    TabPosition pos = index.data(LocationCompleterModel::TabPositionRole).value<TabPosition>();
    if (m_drawSwitchToTab && pos.windowIndex != -1) {
        const QIcon tabIcon = QIcon(":icons/menu/tab.png");
        QRect iconRect(linkRect);
        iconRect.setWidth(m_padding + 16 + m_padding);
        tabIcon.paint(painter, iconRect);

        QRect textRect(linkRect);
        textRect.setX(textRect.x() + m_padding + 16 + m_padding);
        drawTextLine(textRect, LocationCompleterView::tr("Switch to tab"), painter, style, opt, colorLinkRole);
    }
    else {
        drawHighlightedTextLine(linkRect, link, searchText, painter, style, opt, colorLinkRole);
    }

    // Draw line at the very bottom of item if the item is not highlighted
    if (!(opt.state & QStyle::State_Selected)) {
        QRect lineRect(opt.rect.left(), opt.rect.bottom(), opt.rect.width(), 1);
        painter->fillRect(lineRect, opt.palette.color(QPalette::AlternateBase));
    }
}

static bool sizeBiggerThan(const QString &s1, const QString &s2)
{
    return s1.size() > s2.size();
}

void LocationCompleterDelegate::drawHighlightedTextLine(const QRect &rect, const QString &text, const QString &searchText,
        QPainter* painter, const QStyle* style, const QStyleOptionViewItemV4 &option,
        const QPalette::ColorRole &role) const
{
    QList<int> delimiters;
    QStringList searchStrings = searchText.split(QLatin1Char(' '), QString::SkipEmptyParts);

    // Look for longer parts first
    qSort(searchStrings.begin(), searchStrings.end(), sizeBiggerThan);

    foreach(const QString & string, searchStrings) {
        int delimiter = text.indexOf(string, 0, Qt::CaseInsensitive);

        while (delimiter != -1) {
            int start = delimiter;
            int end = delimiter + string.length();

            bool alreadyContains = false;
            for (int i = 0; i < delimiters.count(); ++i) {
                int dStart = delimiters.at(i);
                int dEnd = delimiters.at(++i);

                if (dStart <= start && dEnd >= end) {
                    alreadyContains = true;
                    break;
                }
            }

            if (!alreadyContains) {
                delimiters.append(start);
                delimiters.append(end);
            }

            delimiter = text.indexOf(string, end, Qt::CaseInsensitive);
        }
    }

    // We need to sort delimiters to properly paint all parts that user typed
    qSort(delimiters);

    // If we don't find any match, just paint it without any highlight
    if (delimiters.isEmpty() || delimiters.count() % 2) {
        drawTextLine(rect, text, painter, style, option, role);
        return;
    }

    QFont normalFont = painter->font();
    QFont boldFont = normalFont;
    boldFont.setBold(true);

    QFontMetrics normalMetrics(normalFont);
    QFontMetrics boldMetrics(boldFont);

    int lastEndPos = 0;
    int lastRectPos = rect.left();

    while (!delimiters.isEmpty()) {
        int start = delimiters.takeFirst();
        int end = delimiters.takeFirst();

        const QString &normalPart = text.mid(lastEndPos, start - lastEndPos);
        const QString &boldPart = text.mid(start, end - start);

        lastEndPos = end;

        if (!normalPart.isEmpty()) {
            int width = normalMetrics.width(normalPart);
            QRect nRect = adjustRect(rect, QRect(lastRectPos, rect.top(), width, rect.height()));

            if (nRect.width() > 0) {
                if (text.isRightToLeft()) {
                    nRect = style->visualRect(Qt::RightToLeft, rect, nRect);
                }
                painter->setFont(normalFont);
                drawTextLine(nRect, normalPart, painter, style, option, role);

                lastRectPos += nRect.width();
            }
        }

        if (!boldPart.isEmpty()) {
            int width = boldMetrics.width(boldPart);
            QRect bRect = adjustRect(rect, QRect(lastRectPos, rect.top(), width, rect.height()));

            if (bRect.width() > 0) {
                if (text.isRightToLeft()) {
                    bRect = style->visualRect(Qt::RightToLeft, rect, bRect);
                }
                painter->setFont(boldFont);
                drawTextLine(bRect, boldPart, painter, style, option, role);

                // Paint manually line under text instead of using QFont::underline
                QRect underlineRect(bRect.left(), bRect.top() + boldMetrics.ascent() + 1,
                                    bRect.width(), boldFont.pointSize() > 8 ? 2 : 1);

                painter->fillRect(underlineRect, option.palette.color(role));

                lastRectPos += bRect.width();
            }
        }

        if (delimiters.isEmpty() && lastEndPos != text.size()) {
            const QString &lastText = text.mid(lastEndPos);

            int width = normalMetrics.width(lastText);
            QRect nRect = adjustRect(rect, QRect(lastRectPos, rect.top(), width, rect.height()));
            if (text.isRightToLeft()) {
                nRect = style->visualRect(Qt::RightToLeft, rect, nRect);
            }
            painter->setFont(normalFont);
            drawTextLine(nRect, lastText, painter, style, option, role);
        }
    }
}

// RTL Support
#define LRE QChar(0x202A)
#define RLE QChar(0x202B)
#define PDF QChar(0x202C)

void LocationCompleterDelegate::drawTextLine(const QRect &rect, QString text, QPainter* painter,
        const QStyle* style, const QStyleOptionViewItemV4 &option,
        const QPalette::ColorRole &role) const
{
    if (rect.width() > 0) {
        const Qt::LayoutDirection direction = option.widget ? option.widget->layoutDirection() : QApplication::layoutDirection();
        Qt::LayoutDirection textDirection = text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight;
        Qt::Alignment alignment = textDirection == direction ? Qt::AlignLeft : Qt::AlignRight;

        text.isRightToLeft() ? text.prepend(RLE) : text.prepend(LRE);
        text.append(PDF);

        style->drawItemText(painter, rect, Qt::TextSingleLine | alignment, option.palette, true, text, role);
    }
}

QRect LocationCompleterDelegate::adjustRect(const QRect &original, const QRect &created) const
{
    if (created.left() + created.width() >= original.right()) {
        QRect nRect = created;
        nRect.setWidth(original.right() - created.left());

        return nRect;
    }

    return created;
}

QSize LocationCompleterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (!m_rowHeight) {
        QStyleOptionViewItemV4 opt(option);
        initStyleOption(&opt, index);

        const QWidget* w = opt.widget;
        const QStyle* style = w ? w->style() : QApplication::style();
        const int padding = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;

        QFont titleFont = opt.font;
        titleFont.setPointSize(titleFont.pointSize() + 1);

        m_padding = padding > 3 ? padding : 3;

        const QFontMetrics titleMetrics(titleFont);

        // 2 px bigger space between title and link because of underlining
        m_rowHeight = 2 * m_padding + opt.fontMetrics.leading() + opt.fontMetrics.height() + titleMetrics.height() + 2;
    }

    return QSize(200, m_rowHeight);
}

void LocationCompleterDelegate::drawSwitchToTab(bool enable)
{
    m_drawSwitchToTab = enable;
}

