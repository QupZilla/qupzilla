#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject* parent = 0);
    ~Settings();

    static void createSettings(const QString &fileName);
    static void syncSettings();

    void setValue(const QString &key, const QVariant &defaultValue = QVariant());
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());

    void beginGroup(const QString &prefix);
    void endGroup();

signals:

public slots:

private:
    static QSettings* m_settings;

};

#endif // SETTINGS_H
