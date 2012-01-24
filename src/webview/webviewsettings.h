#ifndef WEBVIEWSETTINGS_H
#define WEBVIEWSETTINGS_H

class WebViewSettings
{
public:
    WebViewSettings();

    static void loadSettings();

    static int defaultZoom;
    static bool newTabAfterActive;
};

#endif // WEBVIEWSETTINGS_H
