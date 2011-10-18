#ifndef PROFILEUPDATER_H
#define PROFILEUPDATER_H

#include <QObject>
#include <QDir>
#include <iostream>

class ProfileUpdater : public QObject
{
    Q_OBJECT
public:
    explicit ProfileUpdater(const QString &profilePath, const QString &dataPath);
    void checkProfile();

signals:

public slots:

private:
    void updateProfile(const QString &current, const QString &profile);
    void copyDataToProfile();

    QString m_profilePath;
    QString m_dataPath;

};

#endif // PROFILEUPDATER_H
