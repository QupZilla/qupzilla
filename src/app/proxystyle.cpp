#include "proxystyle.h"

ProxyStyle::ProxyStyle()
    : QProxyStyle()
{
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    if (hint == QStyle::SH_Menu_Scrollable) {
        return 1;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

//int ProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
//{
//    if (metric == PM_TabBarTabHSpace) {
//        return 8;
//    }

//    return QProxyStyle::pixelMetric(metric, option, widget);
//}
