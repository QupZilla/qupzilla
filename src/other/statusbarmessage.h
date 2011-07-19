#ifndef STATUSBARMESSAGE_H
#define STATUSBARMESSAGE_H

#include <QObject>
#include <QToolTip>
#include "squeezelabelv1.h"

class QupZilla;
class TipLabel;

class TipLabel : public SqueezeLabelV1 {
    Q_OBJECT

public:
    TipLabel(QupZilla* parent);
    void paintEvent(QPaintEvent *ev);
    void show();
    void hide();

private slots:
    void checkMainWindowFocus();

private:
    QTimer* m_timer;
    QupZilla* p_QupZilla;
};

class StatusBarMessage : public QObject
{
    Q_OBJECT
public:
    explicit StatusBarMessage(QupZilla* mainClass);

    void showMessage(const QString &message);
    void clearMessage();

signals:

public slots:

private:
    QupZilla* p_QupZilla;
    TipLabel* m_statusBarText;
};

#endif // STATUSBARMESSAGE_H
