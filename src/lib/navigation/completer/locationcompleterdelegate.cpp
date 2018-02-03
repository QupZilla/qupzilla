/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "locationcompletermodel.h"
#include "locationbar.h"
#include "iconprovider.h"
#include "qzsettings.h"
#include "mainapplication.h"
#include "bookmarkitem.h"

#include <algorithm>

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QTextLayout>

LocationCompleterDelegate::LocationCompleterDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_rowHeight(0)
    , m_padding(0)
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

    // Prepare link font
    QFont linkFont = opt.font;
    linkFont.setPointSize(linkFont.pointSize() - 1);

    const QFontMetrics linkMetrics(linkFont);

    int leftPosition = m_padding * 2;
    int rightPosition = opt.rect.right() - m_padding;

    opt.state |= QStyle::State_Active;

    const QIcon::Mode iconMode = opt.state & QStyle::State_Selected ? QIcon::Selected : QIcon::Normal;

    const QPalette::ColorRole colorRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;
    const QPalette::ColorRole colorLinkRole = opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Link;

#ifdef Q_OS_WIN
    opt.palette.setColor(QPalette::All, QPalette::HighlightedText, opt.palette.color(QPalette::Active, QPalette::Text));
    opt.palette.setColor(QPalette::All, QPalette::Highlight, opt.palette.base().color().darker(108));
#endif

    QPalette textPalette = opt.palette;
    textPalette.setCurrentColorGroup(opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled);

    // Draw background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    const bool isVisitSearchItem = index.data(LocationCompleterModel::VisitSearchItemRole).toBool();
    const bool isSearchSuggestion = index.data(LocationCompleterModel::SearchSuggestionRole).toBool();

    LocationBar::LoadAction loadAction;
    bool isWebSearch = isSearchSuggestion;

    BookmarkItem *bookmark = static_cast<BookmarkItem*>(index.data(LocationCompleterModel::BookmarkItemRole).value<void*>());

    if (isVisitSearchItem) {
        loadAction = LocationBar::loadAction(m_originalText);
        isWebSearch = loadAction.type == LocationBar::LoadAction::Search;
        if (!m_forceVisitItem) {
            bookmark = loadAction.bookmark;
        }
    }

    // Draw icon
    const int iconSize = 16;
    const int iconYPos = center - (iconSize / 2);
    QRect iconRect(leftPosition, iconYPos, iconSize, iconSize);
    QPixmap pixmap = index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconSize);
    if (isSearchSuggestion || (isVisitSearchItem && isWebSearch)) {
        pixmap = QIcon::fromTheme(QSL("edit-find"), QIcon(QSL(":icons/menu/search-icon.svg"))).pixmap(iconSize, iconMode);
    }
    if (isVisitSearchItem && bookmark) {
        pixmap = bookmark->icon().pixmap(iconSize);
    } else if (loadAction.type == LocationBar::LoadAction::Search) {
        if (loadAction.searchEngine.name != LocationBar::searchEngine().name) {
            pixmap = loadAction.searchEngine.icon.pixmap(iconSize);
        }
    }
    painter->drawPixmap(iconRect, pixmap);
    leftPosition = iconRect.right() + m_padding * 2;

    // Draw star to bookmark items
    int starPixmapWidth = 0;
    if (bookmark) {
        const QIcon icon = IconProvider::instance()->bookmarkIcon();
        const QSize starSize(16, 16);
        starPixmapWidth = starSize.width();
        QPoint pos(rightPosition - starPixmapWidth, center - starSize.height() / 2);
        QRect starRect(pos, starSize);
        painter->drawPixmap(starRect, icon.pixmap(starSize, iconMode));
    }

    QString searchText = index.data(LocationCompleterModel::SearchStringRole).toString();

    // Draw title
    leftPosition += 2;
    QRect titleRect(leftPosition, center - opt.fontMetrics.height() / 2, opt.rect.width() * 0.6, opt.fontMetrics.height());
    QString title = index.data(LocationCompleterModel::TitleRole).toString();
    painter->setFont(opt.font);

    if (isVisitSearchItem) {
        if (bookmark) {
            title = bookmark->title();
        } else {
            title = m_originalText.trimmed();
            searchText.clear();
        }
    }

    leftPosition += viewItemDrawText(painter, &opt, titleRect, title, textPalette.color(colorRole), searchText);
    leftPosition += m_padding * 2;

    // Trim link to maximum number of characters that can be visible, otherwise there may be perf issue with huge URLs
    const int maxChars = (opt.rect.width() - leftPosition) / opt.fontMetrics.width(QL1C('i'));
    QString link;
    const QByteArray linkArray = index.data(Qt::DisplayRole).toByteArray();
    if (!linkArray.startsWith("data") && !linkArray.startsWith("javascript")) {
        link = QString::fromUtf8(QByteArray::fromPercentEncoding(linkArray)).left(maxChars);
    } else {
        link = QString::fromLatin1(linkArray.left(maxChars));
    }

    if (isVisitSearchItem || isSearchSuggestion) {
        if (!opt.state.testFlag(QStyle::State_Selected) && !opt.state.testFlag(QStyle::State_MouseOver)) {
            link.clear();
        } else if (isVisitSearchItem && (!isWebSearch || m_forceVisitItem)) {
            link = tr("Visit");
        } else {
            QString searchEngineName = loadAction.searchEngine.name;
            if (searchEngineName.isEmpty()) {
                searchEngineName = LocationBar::searchEngine().name;
            }
            link = tr("Search with %1").arg(searchEngineName);
        }
    }

    if (bookmark) {
        link = bookmark->url().toString();
    }

    // Draw separator
    if (!link.isEmpty()) {
        QChar separator = QL1C('-');
        QRect separatorRect(leftPosition, center - linkMetrics.height() / 2, linkMetrics.width(separator), linkMetrics.height());
        style->drawItemText(painter, separatorRect, Qt::AlignCenter, textPalette, true, separator, colorRole);
        leftPosition += separatorRect.width() + m_padding * 2;
    }

    // Draw link
    const int leftLinkEdge = leftPosition;
    const int rightLinkEdge = rightPosition - m_padding - starPixmapWidth;
    QRect linkRect(leftLinkEdge, center - linkMetrics.height() / 2, rightLinkEdge - leftLinkEdge, linkMetrics.height());
    painter->setFont(linkFont);

    // Draw url (or switch to tab)
    int tabPos = index.data(LocationCompleterModel::TabPositionTabRole).toInt();

    if (qzSettings->showSwitchTab && !m_forceVisitItem && tabPos != -1) {
        const QIcon tabIcon = QIcon(QSL(":icons/menu/tab.svg"));
        QRect iconRect(linkRect);
        iconRect.setX(iconRect.x());
        iconRect.setWidth(16);
        painter->drawPixmap(iconRect, tabIcon.pixmap(iconRect.size(), iconMode));

        QRect textRect(linkRect);
        textRect.setX(textRect.x() + m_padding + 16 + m_padding);
        viewItemDrawText(painter, &opt, textRect, tr("Switch to tab"), textPalette.color(colorLinkRole));
    } else if (isVisitSearchItem || isSearchSuggestion) {
        viewItemDrawText(painter, &opt, linkRect, link, textPalette.color(colorLinkRole));
    } else {
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

        m_padding = padding > 3 ? padding : 3;
        m_rowHeight = 4 * m_padding + qMax(16, opt.fontMetrics.height());
    }

    return QSize(200, m_rowHeight);
}

void LocationCompleterDelegate::setForceVisitItem(bool enable)
{
    m_forceVisitItem = enable;
}

void LocationCompleterDelegate::setOriginalText(const QString &originalText)
{
    m_originalText = originalText;
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
int LocationCompleterDelegate::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect,
                                                const QString &text, const QColor &color, const QString &searchText) const
{
    if (text.isEmpty()) {
        return 0;
    }

    const QFontMetrics fontMetrics(p->font());
    QString elidedText = fontMetrics.elidedText(text, option->textElideMode, rect.width());
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
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

            while (!delimiters.isEmpty()) {
                QTextLayout::FormatRange highlightedPart;
                int start = delimiters.takeFirst();
                int end = delimiters.takeFirst();
                highlightedPart.start = start;
                highlightedPart.length = end - start;
                highlightedPart.format.setFontWeight(QFont::Bold);
                highlightedPart.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);

                highlightParts << highlightedPart;
            }

            textLayout.setAdditionalFormats(highlightParts);
        }
    }

    // do layout
    viewItemTextLayout(textLayout, rect.width());

    if (textLayout.lineCount() <= 0) {
        return 0;
    }

    QTextLine textLine = textLayout.lineAt(0);

    // if elidedText after highlighting is longer
    // than available width then re-elide it and redo layout
    int diff = textLine.naturalTextWidth() - rect.width();
    if (diff > 0) {
        elidedText = fontMetrics.elidedText(elidedText, option->textElideMode, rect.width() - diff);

        textLayout.setText(elidedText);
        // redo layout
        viewItemTextLayout(textLayout, rect.width());

        if (textLayout.lineCount() <= 0) {
            return 0;
        }
        textLine = textLayout.lineAt(0);
    }

    // draw line
    p->setPen(color);
    qreal width = qMax<qreal>(rect.width(), textLayout.lineAt(0).width());
    const QRect &layoutRect = QStyle::alignedRect(option->direction, option->displayAlignment, QSize(int(width), int(textLine.height())), rect);
    const QPointF &position = layoutRect.topLeft();

    textLine.draw(p, position);

    return qMin<int>(rect.width(), textLayout.lineAt(0).naturalTextWidth());
}
