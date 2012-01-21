#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include <QProxyStyle>

class ProxyStyle : public QProxyStyle
{
public:
    explicit ProxyStyle();

    int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
//    int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
};

#endif // PROXYSTYLE_H
