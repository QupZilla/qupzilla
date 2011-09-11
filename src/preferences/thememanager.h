#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QWidget>
#include <QSettings>
#include <QDir>
#include <QListWidgetItem>
#include <QHash>

namespace Ui {
    class ThemeManager;
}

class ThemeManager : public QWidget
{
    Q_OBJECT

public:
    explicit ThemeManager(QWidget* parent);
    ~ThemeManager();

    void save();

private slots:
    void currentChanged();

private:
    struct Theme {
        bool isValid;
        QIcon icon;
        QString name;
        QString author;
        QString shortDescription;
        QString longDescription;
    };

    Theme parseTheme(const QString &name);

    Ui::ThemeManager *ui;
    QString m_activeTheme;
    QHash<QString, Theme> m_themeHash;
};

#endif // THEMEMANAGER_H
