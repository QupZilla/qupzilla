#include "testplugin.h"
#include "qupzilla.h"

PluginSpec TestPlugin::pluginSpec()
{
    PluginSpec spec;
    spec.name = tr("Example Plugin");
    spec.info = tr("Example minimal plugin");
    spec.description = tr("Very simple minimal plugin example");
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

    settingsPath = sPath;
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
    translator->load(":/" + locale);
    return translator;
}

void TestPlugin::showSettings(QWidget *parent)
{
    QDialog* dialog = new QDialog(parent);
    QPushButton* b = new QPushButton("Example Plugin v0.0.1");

    QHBoxLayout* l = new QHBoxLayout(dialog);
    l->addWidget(b);
    dialog->setLayout(l);

    dialog->resize(200, 200);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Example Plugin Settings");
    dialog->setWindowIcon(QIcon(":/qupzilla.png"));

    dialog->show();
}

void TestPlugin::populateWebViewMenu(QMenu* menu, QWebView* view, const QWebHitTestResult &r)
{
    Q_UNUSED(view)
    QString title;
    if (!r.imageUrl().isEmpty()) {
        title += " on image";
    }
    if (!r.linkUrl().isEmpty()) {
        title += " on link";
    }
    QWebElement element = r.element();
    if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea")) {
        title += " on input";
    }
    menu->addAction(tr("My first plugin action") + title, this, SLOT(actionSlot()));
}

void TestPlugin::populateHelpMenu(QMenu* menu)
{
    menu->addAction(tr("My first plugin action"), this, SLOT(actionSlot()));
}

void TestPlugin::populateToolsMenu(QMenu* menu)
{
    menu->addAction(tr("My first plugin action"), this, SLOT(actionSlot()));
}

void TestPlugin::actionSlot()
{
    QMessageBox::information(0, tr("Hello"), tr("First plugin action works :-)"));
}

//Export plugin macro
Q_EXPORT_PLUGIN2(ExamplePlugin, TestPlugin)
