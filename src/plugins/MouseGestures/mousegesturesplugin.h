#ifndef MOUSEGESTURESPLUGIN_H
#define MOUSEGESTURESPLUGIN_H

#include "plugininterface.h"

class MouseGestures;
class MouseGesturesPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    MouseGesturesPlugin();
    PluginSpec pluginSpec();

    void init(const QString &sPath);
    void unload();
    bool testPlugin();

    QTranslator* getTranslator(const QString &locale);
    void showSettings(QWidget* parent = 0);

    bool mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);
    bool mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event);

private:
    MouseGestures* m_gestures;

};

#endif // MOUSEGESTURESPLUGIN_H
