#ifndef CLEARPRIVATEDATA_H
#define CLEARPRIVATEDATA_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QDialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>

class ClickableLabel;
class QupZilla;
class ClearPrivateData : public QDialog
{
    Q_OBJECT
public:
    explicit ClearPrivateData(QupZilla* mainClass, QWidget *parent = 0);

signals:

public slots:

private slots:
    void dialogAccepted();
    void clearFlash();

private:
    QupZilla* p_QupZilla;

    QBoxLayout* m_layout;
    QLabel* m_label;
    QDialogButtonBox* m_buttonBox;

    QCheckBox* m_clearHistory;
    QCheckBox* m_clearCookies;
    QCheckBox* m_clearCache;
    QCheckBox* m_clearIcons;

    ClickableLabel* m_clearFlashCookies;
};

#endif // CLEARPRIVATEDATA_H
