/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2017 David Rosca <nowrep@gmail.com>
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
#include "tabfilterdelegate.h"

#include <QPainter>
#include <QApplication>
#include <QTextLayout>

TabFilterDelegate::TabFilterDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

// most of codes taken from QCommonStyle::drawControl() and add our custom text drawer
void TabFilterDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();

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

    painter->save();
    painter->setClipRect(opt.rect);

    QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, w);
    QRect iconRect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, w);
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, w);

    // draw the background
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // draw the check mark
    if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
        QStyleOptionViewItem opt2(opt);
        opt2.rect = checkRect;
        opt2.state = opt2.state & ~QStyle::State_HasFocus;

        switch (opt.checkState) {
        case Qt::Unchecked:
            opt2.state |= QStyle::State_Off;
            break;
        case Qt::PartiallyChecked:
            opt2.state |= QStyle::State_NoChange;
            break;
        case Qt::Checked:
            opt2.state |= QStyle::State_On;
            break;
        }
        style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt2, painter, w);
    }

    // draw the icon
    QIcon::Mode mode = QIcon::Normal;
    if (!(opt.state & QStyle::State_Enabled))
        mode = QIcon::Disabled;
    else if (opt.state & QStyle::State_Selected)
        mode = QIcon::Selected;
    QIcon::State state = opt.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    opt.icon.paint(painter, iconRect, opt.decorationAlignment, mode, state);

    // draw the text
    if (!opt.text.isEmpty()) {
        const QString filterText = property("filterText").toString();

        QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        if (opt.state & QStyle::State_Selected) {
            painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        } else {
            painter->setPen(opt.palette.color(cg, QPalette::Text));
        }
        if (opt.state & QStyle::State_Editing) {
            painter->setPen(opt.palette.color(cg, QPalette::Text));
            painter->drawRect(textRect.adjusted(0, 0, -1, -1));
        }

        painter->setFont(opt.font);
        viewItemDrawText(painter, &opt, textRect, opt.text, textPalette.color(colorRole), filterText);
    }

    painter->restore();
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
void TabFilterDelegate::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect,
                                                 const QString &text, const QColor &color, const QString &searchText) const
{
    if (text.isEmpty()) {
        return;
    }

    const QWidget* widget = option->widget;
    const bool isRtlLayout = widget ? widget->isRightToLeft() : QApplication::isRightToLeft();
    const QStyle* proxyStyle = widget ? widget->style()->proxy() : QApplication::style()->proxy();
    const int textMargin = proxyStyle->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const QFontMetrics fontMetrics(p->font());
    QString elidedText = fontMetrics.elidedText(text, option->textElideMode, textRect.width());
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    textOption.setTextDirection(text.isRightToLeft() ? Qt::RightToLeft : Qt::LeftToRight);
    textOption.setAlignment(Qt::AlignVCenter | (isRtlLayout ? Qt::AlignRight : Qt::AlignLeft));
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
