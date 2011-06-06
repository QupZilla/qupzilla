#include "buttonwithmenu.h"

ButtonWithMenu::ButtonWithMenu(QWidget* parent) :
    QToolButton(parent)
  , m_menu(new QMenu(this))
  , m_currentItem(0)
{
    setPopupMode(QToolButton::InstantPopup);
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::ClickFocus);
    setMenu(m_menu);
}

void ButtonWithMenu::setCurrentItem()
{
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        setActiveItem(action->data().value<Item>());
    }
}

void ButtonWithMenu::addItem(const Item &item)
{
    m_items.append(item);

    if (!m_currentItem)
        setActiveItem(item);

    QVariant variant;
    variant.setValue<Item>(item);
    m_menu->addAction(item.icon, item.text, this, SLOT(setCurrentItem()))->setData(variant);

    emit itemAdded(item);
}

void ButtonWithMenu::addItems(const QList<Item> &items)
{
    foreach (const Item &item, items) {
        addItem(item);
    }
}

void ButtonWithMenu::removeItem(const Item &item)
{
    int index = m_items.indexOf(item);
    if (index < 0)
        return;

    m_items.removeOne(item);

    if (*m_currentItem == item)
        setActiveItem(m_items.takeFirst());

    generateMenu();
}

void ButtonWithMenu::setActiveItem(const Item &item)
{
    int index = m_items.indexOf(item);
    if (index < 0)
        return;

    m_currentItem = const_cast<Item*>(&m_items.at(index));

    setIcon(m_currentItem->icon);
    setToolTip(m_currentItem->text);

    emit activeItemChanged(*m_currentItem);
}

void ButtonWithMenu::wheelEvent(QWheelEvent *event)
{
    int currItemIndex = m_items.indexOf(*m_currentItem);
    int itemsCount = m_items.count();

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps == 1) {
        if (currItemIndex != 0)
            setActiveItem(m_items.at(currItemIndex - 1));
    } else {
        if (currItemIndex < itemsCount - 1)
            setActiveItem(m_items.at(currItemIndex + 1));
    }
    event->accept();
}

ButtonWithMenu::Item* ButtonWithMenu::activeItem()
{
    return m_currentItem;
}

void ButtonWithMenu::generateMenu()
{
    m_menu->clear();
    addItems(m_items);
}

ButtonWithMenu::~ButtonWithMenu()
{
    delete m_menu;
}
