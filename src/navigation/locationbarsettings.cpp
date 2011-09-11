#include "locationbarsettings.h"
#include "mainapplication.h"

LocationBarSettings* LocationBarSettings::s_instance = 0;
bool LocationBarSettings::selectAllOnDoubleClick = false;
bool LocationBarSettings::addComWithCtrl = false;
bool LocationBarSettings::addCountryWithAlt = false;

LocationBarSettings::LocationBarSettings()
{
    loadSettings();
}

LocationBarSettings* LocationBarSettings::instance()
{
    if (!s_instance)
        s_instance = new LocationBarSettings();
    return s_instance;
}

void LocationBarSettings::loadSettings()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("AddressBar");
    selectAllOnDoubleClick = settings.value("SelectAllTextOnDoubleClick",true).toBool();
    addComWithCtrl = settings.value("AddComDomainWithCtrlKey",false).toBool();
    addCountryWithAlt = settings.value("AddCountryDomainWithAltKey",true).toBool();
}
