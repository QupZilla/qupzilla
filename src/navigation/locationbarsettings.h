#ifndef LOCATIONBARSETTINGS_H
#define LOCATIONBARSETTINGS_H

#include <QSettings>

class LocationBarSettings
{
public:
    LocationBarSettings();

    static LocationBarSettings* instance();

    static void loadSettings();
    static bool selectAllOnDoubleClick;
    static bool addComWithCtrl;
    static bool addCountryWithAlt;

private:
    static LocationBarSettings* s_instance;
};

#endif // LOCATIONBARSETTINGS_H
