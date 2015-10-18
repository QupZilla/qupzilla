#ifndef TABMANAGERSETTINGS_H
#define TABMANAGERSETTINGS_H

#include <QDialog>

namespace Ui {
class TabManagerSettings;
}
class TabManagerPlugin;

class TabManagerSettings : public QDialog
{
    Q_OBJECT

public:
    explicit TabManagerSettings(TabManagerPlugin* plugin, QWidget *parent = 0);
    ~TabManagerSettings();

public slots:
    void accept();

private:
    Ui::TabManagerSettings* ui;
    TabManagerPlugin* m_plugin;
};

#endif // TABMANAGERSETTINGS_H
