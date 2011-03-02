#include "testplugin.h"

void TestPlugin::init(QString sPath)
{
    settingsPath = sPath;
    //This function is called right after plugin is loaded
    qDebug() << __FUNCTION__ << "called";
}

bool TestPlugin::testPlugin()
{
    //This function is loaded right after init()
    //There should be some testing if plugin is loaded correctly
    //If this function returns false, plugin is automatically unloaded

    return true;
}

QTranslator* TestPlugin::getTranslator(QString locale)
{
    QTranslator* translator = new QTranslator();
    translator->load(":/"+locale);
    return translator;
}

void TestPlugin::showSettings()
{
    QWidget* widget = new QWidget();
    new QLabel("Example Plugin v0.0.1", widget);
    widget->resize(200,200);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setWindowModality(Qt::WindowModal); //As the preferences window is modal too
    widget->setWindowTitle("Example Plugin Settings");
    widget->setWindowIcon(pluginIcon());
    widget->show();
}

void TestPlugin::populateWebViewMenu(QMenu *menu, QWebView *view, QWebHitTestResult r)
{
    Q_UNUSED(view)
    QString title;
    if (!r.imageUrl().isEmpty())
        title += " on image";
    if (!r.linkUrl().isEmpty())
        title += " on link";
    QWebElement element = r.element();
       if (!element.isNull() && (element.tagName().toLower() == "input" || element.tagName().toLower() == "textarea"))
           title += " on input";
    menu->addAction(tr("My first plugin action") + title, this, SLOT(actionSlot()));
}

void TestPlugin::populateHelpMenu(QMenu *menu)
{
    menu->addAction(tr("My first plugin action"), this, SLOT(actionSlot()));
}

void TestPlugin::populateToolsMenu(QMenu *menu)
{
    menu->addAction(tr("My first plugin action"), this, SLOT(actionSlot()));
}

void TestPlugin::actionSlot()
{
    QMessageBox::information(0, tr("Hello"), tr("First plugin action works :-)"));
}

//Export plugin macro
Q_EXPORT_PLUGIN2(ExamplePlugin, TestPlugin)
