#ifndef PLUGINLISTDELEGATE_H
#define PLUGINLISTDELEGATE_H

#include <QItemDelegate>
#include <QPainter>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextBlock>
#include <QListWidget>
#include <QApplication>

class PluginListDelegate : public QItemDelegate
{
public:
    PluginListDelegate(QListWidget* parent);

    void drawDisplay(QPainter* painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QListWidget* m_listWidget;
};

#endif // PLUGINLISTDELEGATE_H
