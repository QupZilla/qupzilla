#include "mousegesturesplugin.h"
#include "pluginproxy.h"
#include "mousegestures.h"
#include "qupzilla.h"

MouseGesturesPlugin::MouseGesturesPlugin()
    : QObject()
    , m_gestures(0)
{
}

PluginSpec MouseGesturesPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Mouse Gestures";
    spec.info = "Mouse gestures for QupZilla";
    spec.description = "Provides support for navigating by mouse gestures through webpages";
    spec.version = "0.1.0";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QIcon(":/mousegestures/data/icon.png");
    spec.hasSettings = true;

    return spec;
}

void MouseGesturesPlugin::init(const QString &sPath)
{
    Q_UNUSED(sPath)

    m_gestures = new MouseGestures(this);

    QZ_REGISTER_EVENT_HANDLER(PluginProxy::MousePressHandler);
    QZ_REGISTER_EVENT_HANDLER(PluginProxy::MouseReleaseHandler);
    QZ_REGISTER_EVENT_HANDLER(PluginProxy::MouseMoveHandler);
}

void MouseGesturesPlugin::unload()
{
    m_gestures->deleteLater();
}

bool MouseGesturesPlugin::testPlugin()
{
    // Let's be sure, require latest version of QupZilla

    return (QupZilla::VERSION == "1.1.8");
}

QTranslator* MouseGesturesPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator();
    translator->load(":/mousegestures/locale/" + locale);
    return translator;
}

void MouseGesturesPlugin::showSettings(QWidget* parent)
{
    m_gestures->showSettings(parent);
}


bool MouseGesturesPlugin::mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        m_gestures->mousePress(obj, event);
    }

    return false;
}

bool MouseGesturesPlugin::mouseRelease(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        return m_gestures->mouseRelease(obj, event);
    }

    return false;
}

bool MouseGesturesPlugin::mouseMove(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    if (type == Qz::ON_WebView) {
        m_gestures->mouseMove(obj, event);
    }

    return false;
}

Q_EXPORT_PLUGIN2(MouseGestures, MouseGesturesPlugin)
