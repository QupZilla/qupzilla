#include "webviewsettings.h"
#include "settings.h"

int WebViewSettings::defaultZoom = 100;

WebViewSettings::WebViewSettings()
{
}

void WebViewSettings::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    defaultZoom = settings.value("DefaultZoom", 100).toInt();
    settings.endGroup();
}

