#ifndef BUTTONBOX_H
#define BUTTONBOX_H

#include <QDialogButtonBox>
#include <QAbstractButton>

class ButtonBox : public QDialogButtonBox
{
    Q_OBJECT
public:
    explicit ButtonBox(QWidget *parent = 0);
    ButtonRole clickedButtonRole();

signals:

public slots:

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    QDialogButtonBox::ButtonRole m_clickedButton;

};

#endif // BUTTONBOX_H
