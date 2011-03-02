#ifndef PREFERENCES_H
#define PREFERENCES_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QDialog>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QColorDialog>
#include <QAbstractButton>

namespace Ui {
    class Preferences;
}

class AutoFillManager;
class QupZilla;
class PluginsList;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QupZilla* mainClass, QWidget *parent = 0);
    ~Preferences();

private slots:
    void saveSettings();
    void buttonClicked(QAbstractButton* button);

    void showStackedPage(QListWidgetItem* item);
    void newTabChanged();
    void chooseDownPath();
    void showCookieManager();
    void chooseBackgroundPath();
    void useActualHomepage();
    void useActualNewTab();
    void resetBackground();
    void chooseColor();

    void allowJavaScriptChanged(bool stat);
    void saveHistoryChanged(bool stat);
    void saveCookiesChanged(bool stat);
    void downLocChanged(bool state);
    void allowCacheChanged(bool state);
    void showPassManager(bool state);
    void useBgImageChanged(bool state);
    void cacheValueChanged(int value);
    void pageCacheValueChanged(int value);

private:
    void updateBgLabel();
    Ui::Preferences *ui;
    QupZilla* p_QupZilla;
    AutoFillManager* m_autoFillManager;
    PluginsList* m_pluginsList;

    QColor m_menuTextColor;
    QString m_homepage;
    QString m_newTabUrl;
    int m_afterLaunch;
    int m_onNewTab;
    QSize m_bgLabelSize;
};

#endif // PREFERENCES_H
