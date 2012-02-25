#include "pluginlistdelegate.h"

PluginListDelegate::PluginListDelegate(QListWidget* parent)
    : QItemDelegate(parent)
    , m_listWidget(parent)
{
}

void PluginListDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    QTextDocument textDocument;
    textDocument.setHtml(text);

    QTextLayout textLayout(textDocument.begin());
    textLayout.setFont(option.font);

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding

    textLayout.beginLayout();
    qreal height = 0;
    while (1) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(textRect.width());
        height += 3;
        line.setPosition(QPoint(0, height));
        height += line.height();
    }
    textLayout.endLayout();

    textLayout.draw(painter, QPointF(textRect.left(), textRect.top()));
}

QSize PluginListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QItemDelegate::sizeHint(option, index);
    size.setWidth(m_listWidget->width() - 5);
    size.setHeight(option.fontMetrics.height() * 4);

    return size;
}
