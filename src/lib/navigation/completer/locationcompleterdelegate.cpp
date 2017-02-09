/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "qzsettings.h"

#include <algorithm>

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QTextLayout>

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
    QStyleOptionViewItem opt = option;
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
    } else {
        opt.state &= ~QStyle::State_Selected;
    }

    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;
    const QPalette::ColorRole colorLinkRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Link;

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
        const QIcon icon = IconProvider::instance()->bookmarkIcon();
        const QSize starSize(16, 16);
        starPixmapWidth = starSize.width();
        QPoint pos(rightPosition - starPixmapWidth, center - starSize.height() / 2);
        QRect starRect(pos, starSize);
        painter->drawPixmap(starRect, icon.pixmap(starSize));
    }

    const QString searchText = index.data(LocationCompleterModel::SearchStringRole).toString();

    // Draw title
    const int leftTitleEdge = leftPosition + 2;
    // RTL Support: remove conflicting of right-aligned text and starpixmap!
    const int rightTitleEdge = rightPosition - m_padding - starPixmapWidth;
    QRect titleRect(leftTitleEdge, opt.rect.top() + m_padding, rightTitleEdge - leftTitleEdge, titleMetrics.height());
    QString title = index.data(LocationCompleterModel::TitleRole).toString();
    painter->setFont(titleFont);

    viewItemDrawText(painter, &opt, titleRect, title, textPalette.color(colorRole), searchText);

    // Draw link
    const int infoYPos = titleRect.bottom() + opt.fontMetrics.leading() + 2;
    QRect linkRect(titleRect.x(), infoYPos, titleRect.width(), opt.fontMetrics.height());
    const QByteArray linkArray = index.data(Qt::DisplayRole).toByteArray();

    // Let's assume that more than 500 characters won't fit in line on any display...
    // Fixes performance when trying to get elidedText for a really long
    // (length() > 1000000) urls - data: urls can get that long

    QString link;
    if (!linkArray.startsWith("data") && !linkArray.startsWith("javascript")) {
        link = QString::fromUtf8(QByteArray::fromPercentEncoding(linkArray)).left(500);
    }
    else {
        link = QString::fromLatin1(linkArray.left(500));
    }

    painter->setFont(opt.font);

    // Draw url (or switch to tab)
    int tabPos = index.data(LocationCompleterModel::TabPositionTabRole).toInt();

    if (drawSwitchToTab() && tabPos != -1) {
        const QIcon tabIcon = QIcon(QSL(":icons/menu/tab.svg"));
        QRect iconRect(linkRect);
        iconRect.setX(iconRect.x() + m_padding * 2);
        iconRect.setWidth(16);
        tabIcon.paint(painter, iconRect);

        QRect textRect(linkRect);
        textRect.setX(textRect.x() + m_padding + 16 + m_padding);
        viewItemDrawText(painter, &opt, textRect, LocationCompleterView::tr("Switch to tab"), textPalette.color(colorLinkRole));
    }
    else {
        viewItemDrawText(painter, &opt, linkRect, link, textPalette.color(colorLinkRole), searchText);
    }

    // Draw line at the very bottom of item if the item is not highlighted
    if (!(opt.state & QStyle::State_Selected)) {
        QRect lineRect(opt.rect.left(), opt.rect.bottom(), opt.rect.width(), 1);
        painter->fillRect(lineRect, opt.palette.color(QPalette::AlternateBase));
    }
}

QSize LocationCompleterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    if (!m_rowHeight) {
        QStyleOptionViewItem opt(option);
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

void LocationCompleterDelegate::setShowSwitchToTab(bool enable)
{
    m_drawSwitchToTab = enable;
}

bool LocationCompleterDelegate::drawSwitchToTab() const
{
    return qzSettings->showSwitchTab && m_drawSwitchToTab;
}

static bool sizeBiggerThan(const QString &s1, const QString &s2)
{
    return s1.size() > s2.size();
}

static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth)
{
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    QTextLine line = textLayout.createLine();
    if (line.isValid()) {
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());

        textLayout.endLayout();
    }
    return QSizeF(widthUsed, height);
}

// most of codes taken from QCommonStylePrivate::viewItemDrawText()
// added highlighting and simplified for single-line textlayouts
void LocationCompleterDelegate::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect,
                                                 const QString &text, const QColor &color, const QString &searchText) const
{
    if (text.isEmpty()) {
        return;
    }

    const QWidget* widget = option->widget;
    const QStyle* proxyStyle = widget ? widget->style()->proxy() : QApplication::style()->proxy();
    const int textMargin = proxyStyle->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const QFontMetrics fontMetrics(p->font());
    QString elidedText = fontMetrics.elidedText(text, option->textElideMode, textRect.width());
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setTextDirection(text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
    textOption.setAlignment(QStyle::visualAlignment(textOption.textDirection(), option->displayAlignment));
    QTextLayout textLayout;
    textLayout.setFont(p->font());
    textLayout.setText(elidedText);
    textLayout.setTextOption(textOption);

    if (!searchText.isEmpty()) {
        QList<int> delimiters;
        QStringList searchStrings = searchText.split(QLatin1Char(' '), QString::SkipEmptyParts);
        // Look for longer parts first
        std::sort(searchStrings.begin(), searchStrings.end(), sizeBiggerThan);

        foreach (const QString &string, searchStrings) {
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
        std::sort(delimiters.begin(), delimiters.end());

        // If we don't find any match, just paint it without any highlight
        if (!delimiters.isEmpty() && !(delimiters.count() % 2)) {
            QList<QTextLayout::FormatRange> highlightParts;

            QTextLayout::FormatRange lighterWholeLine;
            lighterWholeLine.start = 0;
            lighterWholeLine.length = elidedText.size();
            QColor lighterColor = color.lighter(130);
            if (lighterColor == color) {
                lighterColor = QColor(Qt::gray).darker(180);
            }
            lighterWholeLine.format.setForeground(lighterColor);
            highlightParts << lighterWholeLine;

            while (!delimiters.isEmpty()) {
                QTextLayout::FormatRange highlightedPart;
                int start = delimiters.takeFirst();
                int end = delimiters.takeFirst();
                highlightedPart.start = start;
                highlightedPart.length = end - start;
                highlightedPart.format.setFontWeight(QFont::Bold);
                highlightedPart.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                highlightedPart.format.setForeground(color);

                highlightParts << highlightedPart;
            }

            textLayout.setAdditionalFormats(highlightParts);
        }
    }

    // do layout
    viewItemTextLayout(textLayout, textRect.width());

    if (textLayout.lineCount() <= 0) {
        return;
    }

    QTextLine textLine = textLayout.lineAt(0);

    // if elidedText after highlighting is longer
    // than available width then re-elide it and redo layout
    int diff = textLine.naturalTextWidth() - textRect.width();
    if (diff > 0) {
        elidedText = fontMetrics.elidedText(elidedText, option->textElideMode, textRect.width() - diff);

        textLayout.setText(elidedText);
        // redo layout
        viewItemTextLayout(textLayout, textRect.width());

        if (textLayout.lineCount() <= 0) {
            return;
        }
        textLine = textLayout.lineAt(0);
    }

    // draw line
    p->setPen(color);
    qreal width = qMax<qreal>(textRect.width(), textLayout.lineAt(0).width());
    const QRect &layoutRect = QStyle::alignedRect(option->direction, option->displayAlignment, QSize(int(width), int(textLine.height())), textRect);
    const QPointF &position = layoutRect.topLeft();

    textLine.draw(p, position);
}
