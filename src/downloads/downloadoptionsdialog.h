#ifndef DOWNLOADOPTIONSDIALOG_H
#define DOWNLOADOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
    class DownloadOptionsDialog;
}

class DownloadOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadOptionsDialog(QWidget *parent = 0);
    ~DownloadOptionsDialog();

private:
    Ui::DownloadOptionsDialog *ui;
};

#endif // DOWNLOADOPTIONSDIALOG_H
