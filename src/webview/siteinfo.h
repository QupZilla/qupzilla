#ifndef SITEINFO_H
#define SITEINFO_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QDialog>

namespace Ui {
    class SiteInfo;
}

class QupZilla;
class SiteInfo : public QDialog
{
    Q_OBJECT

public:
    explicit SiteInfo(QupZilla* mainClass, QWidget *parent = 0);
    ~SiteInfo();

private:
    Ui::SiteInfo *ui;
    QupZilla* p_QupZilla;
};

#endif // SITEINFO_H
