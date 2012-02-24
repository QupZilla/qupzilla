#ifndef MOUSEGESTURESSETTINGSDIALOG_H
#define MOUSEGESTURESSETTINGSDIALOG_H

#include <QDialog>
#include <QTextBrowser>

namespace Ui
{
class MouseGesturesSettingsDialog;
}

class MouseGesturesSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MouseGesturesSettingsDialog(QWidget* parent = 0);
    ~MouseGesturesSettingsDialog();

private slots:
    void showLicense();

private:
    Ui::MouseGesturesSettingsDialog* ui;
};

#endif // MOUSEGESTURESSETTINGSDIALOG_H
