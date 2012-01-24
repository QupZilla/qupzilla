#include "webviewsettings.h"
#include "settings.h"

int WebViewSettings::defaultZoom = 100;
bool WebViewSettings::newTabAfterActive = true;


WebViewSettings::WebViewSettings()
{
}

void WebViewSettings::loadSettings()
{
    Settings settings;
    settings.beginGroup("Browser-Tabs-Settings");
    newTabAfterActive = settings.value("newTabAfterActive", true).toBool();
    settings.endGroup();

    settings.beginGroup("Web-Browser-Settings");
    defaultZoom = settings.value("DefaultZoom", 100).toInt();
    settings.endGroup();
}

