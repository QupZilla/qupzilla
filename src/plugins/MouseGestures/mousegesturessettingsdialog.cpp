#include "mousegesturessettingsdialog.h"
#include "ui_mousegesturessettingsdialog.h"
#include "globalfunctions.h"

MouseGesturesSettingsDialog::MouseGesturesSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MouseGesturesSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->licenseButton, SIGNAL(clicked()), this, SLOT(showLicense()));
}

MouseGesturesSettingsDialog::~MouseGesturesSettingsDialog()
{
    delete ui;
}

void MouseGesturesSettingsDialog::showLicense()
{
    QTextBrowser* b = new QTextBrowser();
    b->setAttribute(Qt::WA_DeleteOnClose);
    b->setWindowTitle(tr("License Viewer"));
    b->resize(450, 500);
    b->setText(qz_readAllFileContents(":mousegestures/data/copyright"));
    qz_centerWidgetOnScreen(b);
    b->show();
}
