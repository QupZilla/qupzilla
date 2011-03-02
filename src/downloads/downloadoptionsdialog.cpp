#include "downloadoptionsdialog.h"
#include "ui_downloadoptionsdialog.h"

DownloadOptionsDialog::DownloadOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadOptionsDialog)
{
    ui->setupUi(this);
}

DownloadOptionsDialog::~DownloadOptionsDialog()
{
    delete ui;
}
