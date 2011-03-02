#ifndef WEBSEARCHBAR_H
#define WEBSEARCHBAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include "lineedit.h"

class QupZilla;
class LineEdit;
class ClickableLabel;
class WebSearchBar : public LineEdit
{
    Q_OBJECT;
public:
    explicit WebSearchBar(QupZilla* mainClass, QWidget *parent = 0);

private slots:
    void searchChanged();
    void search();

private:
    ClickableLabel* m_buttonSearch;
    QToolButton* m_boxSearchType;

    void setupSearchTypes();
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);

    QupZilla* p_QupZilla;
};

#endif // WEBSEARCHBAR_H
