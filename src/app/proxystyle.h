#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include <QProxyStyle>

class ProxyStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit ProxyStyle();

    int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
//    int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;

signals:

public slots:

};

#endif // PROXYSTYLE_H
