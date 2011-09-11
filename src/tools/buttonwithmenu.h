#ifndef BUTTONWITHMENU_H
#define BUTTONWITHMENU_H

#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QWheelEvent>

#include "toolbutton.h"

class ButtonWithMenu : public ToolButton
{
    Q_OBJECT
public:
    struct Item {
        QString text;
        QIcon icon;
        QVariant userData;

        Item(const QString &a = QString(), const QIcon &b = QIcon())
        {
            text = a;
            icon = b;
        }

        bool operator==(const Item &a)
        {
            return (a.text == text) && (a.icon.pixmap(16,16).toImage() == icon.pixmap(16,16).toImage());
        }
    };

    explicit ButtonWithMenu(QWidget* parent = 0);
    ~ButtonWithMenu();

    void addItem(const Item &item);
    void addItems(const QList<Item> &items);
    void removeItem(const Item &item);
    void setActiveItem(const Item &item);

    Item* activeItem();

signals:
    void activeItemChanged(const ButtonWithMenu::Item &item);
    void itemAdded(const ButtonWithMenu::Item &item);
    void itemRemoved(const ButtonWithMenu::Item &item);

public slots:

private slots:
    void setCurrentItem();

private:
    void wheelEvent(QWheelEvent *event);
    void generateMenu();

    QMenu* m_menu;
    QList<Item> m_items;
    Item* m_currentItem;

};

Q_DECLARE_METATYPE(ButtonWithMenu::Item)

#endif // BUTTONWITHMENU_H
