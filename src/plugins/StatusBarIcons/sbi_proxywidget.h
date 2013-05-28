#ifndef SBI_PROXYWIDGET_H
#define SBI_PROXYWIDGET_H

#include <QWidget>

namespace Ui
{
class SBI_ProxyWidget;
}

class SBI_NetworkProxy;

class SBI_ProxyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SBI_ProxyWidget(QWidget* parent = 0);
    ~SBI_ProxyWidget();

    SBI_NetworkProxy* getProxy() const;
    void setProxy(const SBI_NetworkProxy &proxy);

    void clear();

private slots:
    void useHttpsProxyChanged(bool enable);

private:
    Ui::SBI_ProxyWidget* ui;
};

#endif // SBI_PROXYWIDGET_H
