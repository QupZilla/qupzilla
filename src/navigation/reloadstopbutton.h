#ifndef RELOADSTOPBUTTON_H
#define RELOADSTOPBUTTON_H

#include <QHBoxLayout>

#include "toolbutton.h"

class ReloadStopButton : public QWidget
{
    Q_OBJECT
public:
    explicit ReloadStopButton(QWidget *parent = 0);
    ~ReloadStopButton();

    void showStopButton();
    void showReloadButton();

    ToolButton* buttonStop() { return m_buttonStop; }
    ToolButton* buttonReload() { return m_buttonReload; }

signals:

public slots:

private:
    ToolButton* m_buttonStop;
    ToolButton* m_buttonReload;
};

#endif // RELOADSTOPBUTTON_H
