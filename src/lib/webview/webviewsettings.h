#ifndef WEBVIEWSETTINGS_H
#define WEBVIEWSETTINGS_H

#include "qz_namespace.h"

class WebViewSettings
{
public:
    WebViewSettings();

    static void loadSettings();

    static int defaultZoom;
};

#endif // WEBVIEWSETTINGS_H
