#include "testplugin.h"
#include "qupzilla.h"
#include "webview.h"
#include "pluginproxy.h"

PluginSpec TestPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = "Example Plugin";
    spec.info = "Example minimal plugin";
    spec.description = "Very simple minimal plugin example";
    spec.version = "0.0.1";
    spec.author = "David Rosca <nowrep@gmail.com>";
    spec.icon = QIcon(":qupzilla.png");
    spec.hasSettings = true;

    return spec;
}

void TestPlugin::init(const QString &sPath)
{
    qDebug() << __FUNCTION__ << "called";

    // This function is called right after plugin is loaded
    // it will be called even if we return false from testPlugin()
    // so it is recommended not to call any QupZilla function here

    m_settingsPath = sPath;
    m_view = 0;

    QZ_REGISTER_EVENT_HANDLER(PluginProxy::MousePressHandler);
}

void TestPlugin::unload()
{
    qDebug() << __FUNCTION__ << "called";

    // This function will be called when unloading plugin
    // it will be also called if we return false from testPlugin()
}

bool TestPlugin::testPlugin()
{
    //This function is called right after init()
    //There should be some testing if plugin is loaded correctly
    //If this function returns false, plugin is automatically unloaded

    return (QupZilla::VERSION == "1.1.8");
}

QTranslator* TestPlugin::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator();
    translator->load(":/testplugin/locale/" + locale);
    return translator;
}

void TestPlugin::showSettings(QWidget* parent)
{
    QDialog* dialog = new QDialog(parent);
    QPushButton* b = new QPushButton("Example Plugin v0.0.1");
    QPushButton* closeButton = new QPushButton(tr("Close"));
    QLabel* label = new QLabel();
    label->setPixmap(QPixmap(":icons/other/about.png"));

    QVBoxLayout* l = new QVBoxLayout(dialog);
    l->addWidget(label);
    l->addWidget(b);
    l->addWidget(closeButton);
    dialog->setLayout(l);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Example Plugin Settings"));
    dialog->setWindowIcon(QIcon(":qupzilla.png"));
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(close()));

    dialog->show();
}

void TestPlugin::populateWebViewMenu(QMenu* menu, WebView* view, const QWebHitTestResult &r)
{
    m_view = view;

    QString title;
    if (!r.imageUrl().isEmpty()) {
        title += " on image";
    }

    if (!r.linkUrl().isEmpty()) {
        title += " on link";
    }

    if (r.isContentEditable()) {
        title += " on input";
    }

    menu->addAction(tr("My first plugin action") + title, this, SLOT(actionSlot()));
}

bool TestPlugin::mousePress(const Qz::ObjectName &type, QObject* obj, QMouseEvent* event)
{
    qDebug() << "mousePress" << type << obj << event;

    return false;
}

void TestPlugin::actionSlot()
{
    QMessageBox::information(m_view, tr("Hello"), tr("First plugin action works :-)"));
}

//Export plugin macro
Q_EXPORT_PLUGIN2(ExamplePlugin, TestPlugin)
