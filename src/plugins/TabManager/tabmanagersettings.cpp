#include "tabmanagersettings.h"
#include "ui_tabmanagersettings.h"
#include "tabmanagerplugin.h"

TabManagerSettings::TabManagerSettings(TabManagerPlugin* plugin, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabManagerSettings),
    m_plugin(plugin)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->sidebarRadio->setChecked(m_plugin->viewType() == TabManagerPlugin::ShowAsSideBar);
    ui->windowRadio->setChecked(m_plugin->viewType() != TabManagerPlugin::ShowAsSideBar);
    ui->checkBox->setChecked(m_plugin->asTabBarReplacement());

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

TabManagerSettings::~TabManagerSettings()
{
    delete ui;
}

void TabManagerSettings::accept()
{
    m_plugin->setViewType(ui->sidebarRadio->isChecked() ? TabManagerPlugin::ShowAsSideBar : TabManagerPlugin::ShowAsWindow);
    m_plugin->setAsTabBarReplacement(ui->checkBox->isChecked());

    QDialog::accept();
}
