#ifndef AUTOSAVER_H
#define AUTOSAVER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#ifdef QT_NO_DEBUG
#ifdef DEVELOPING
#error "TRYING TO RELEASE WITH DEVELOPING FLAG"
#endif
#endif

#include <QObject>
#include <QBasicTimer>
#include <QDebug>

class MainApplication;
class AutoSaver : public QObject
{
    Q_OBJECT
public:
    explicit AutoSaver(QObject *parent = 0);

signals:
    void saveApp();

public slots:

private:
    void timerEvent(QTimerEvent *);
    MainApplication* p_mainApp;
    QBasicTimer m_timer;

};

#endif // AUTOSAVER_H
