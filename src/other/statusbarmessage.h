#ifndef STATUSBARMESSAGE_H
#define STATUSBARMESSAGE_H

#include <QObject>
#include <QToolTip>

class QupZilla;
class TipLabel;
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
