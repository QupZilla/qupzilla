#include "buttonbox.h"

ButtonBox::ButtonBox(QWidget *parent) :
    QDialogButtonBox(parent)
  , m_clickedButton(QDialogButtonBox::RejectRole)
{
    connect(this, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

void ButtonBox::buttonClicked(QAbstractButton *button)
{
    m_clickedButton = buttonRole(button);
}

QDialogButtonBox::ButtonRole ButtonBox::clickedButtonRole()
{
    return m_clickedButton;
}
