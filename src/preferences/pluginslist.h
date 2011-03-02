#ifndef PLUGINSLIST_H
#define PLUGINSLIST_H
#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QListWidgetItem>
#include <QInputDialog>

namespace Ui {
    class PluginsList;
}

class PluginsList : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsList(QWidget *parent = 0);
    ~PluginsList();
    void save();

public slots:
    void reloadPlugins();

private slots:
    //App extension
    void settingsClicked();
    void currentChanged(QListWidgetItem* item);
    void allowAppPluginsChanged(bool state);
    //WebKit plugins
    void addWhitelist();
    void removeWhitelist();
    void allowC2FChanged(bool state);

private:
    void refresh();
    Ui::PluginsList *ui;
};

#endif // PLUGINSLIST_H
