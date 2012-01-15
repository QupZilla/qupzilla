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
