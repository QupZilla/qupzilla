#ifndef MENU_H
#define MENU_H

#include <QMenu>
#include <QMouseEvent>

class Menu : public QMenu
{
    Q_OBJECT
public:
    explicit Menu(QWidget* parent = 0);
    explicit Menu(const QString &title, QWidget* parent = 0);

signals:

public slots:

private:
    void mouseReleaseEvent(QMouseEvent* e);

};

class Action : public QAction
{
    Q_OBJECT
public:
    explicit Action(QObject* parent = 0);
    explicit Action(const QString &text, QObject* parent = 0);
    explicit Action(const QIcon &icon, const QString &text, QObject* parent = 0);

signals:
    void middleClicked();

public slots:
    void triggerMiddleClick();

};

#endif // MENU_H
