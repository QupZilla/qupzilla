#ifndef DOCKTITLEBARWIDGET_H
#define DOCKTITLEBARWIDGET_H

#include <QWidget>

#include "ui_docktitlebarwidget.h"

class DockTitleBarWidget : public QWidget, public Ui_DockTitleBarWidget
{
    Q_OBJECT

public:
    explicit DockTitleBarWidget(const QString &title, QWidget* parent = 0);
    ~DockTitleBarWidget();

    void setTitle(const QString &title);

private:
};

#endif // DOCKTITLEBARWIDGET_H
