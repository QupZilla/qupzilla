#include "settings.h"

QSettings* Settings::m_settings = 0;

Settings::Settings()
{
}

void Settings::createSettings(const QString &fileName)
{
    m_settings = new QSettings(fileName, QSettings::IniFormat);
}

void Settings::syncSettings()
{
    m_settings->sync();
}

void Settings::setValue(const QString &key, const QVariant &defaultValue)
{
    m_settings->setValue(key, defaultValue);
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    return m_settings->value(key, defaultValue);
}

void Settings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

void Settings::endGroup()
{
    m_settings->endGroup();
}

Settings::~Settings()
{
    if (!m_settings->group().isEmpty()) {
        m_settings->endGroup();
    }
}
