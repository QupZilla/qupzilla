/* ============================================================
* Copyright (C) 2012-2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* This file is part of QupZilla - WebKit based browser 2010-2014
* by  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "registerqappassociation.h"
#include "mainapplication.h"
#include "browserwindow.h"

#include "ShlObj.h"
#include <QMessageBox>
#include <QStringList>
#include <QSettings>
#include <QDir>

RegisterQAppAssociation::RegisterQAppAssociation(QObject* parent) :
    QObject(parent)
{
    setPerMachineRegisteration(false);
}

RegisterQAppAssociation::RegisterQAppAssociation(const QString &appRegisteredName, const QString &appPath, const QString &appIcon,
        const QString &appDesc, QObject* parent)
    : QObject(parent)
{
    setPerMachineRegisteration(false);
    setAppInfo(appRegisteredName, appPath, appIcon, appDesc);
}

RegisterQAppAssociation::~RegisterQAppAssociation()
{
}

void RegisterQAppAssociation::addCapability(const QString &assocName, const QString &progId,
        const QString &desc, const QString &iconPath, AssociationType type)
{
    _assocDescHash.insert(progId, QPair<QString, QString>(desc, QDir::toNativeSeparators(iconPath)));
    switch (type) {
    case FileAssociation:
        _fileAssocHash.insert(assocName, progId);
        break;
    case UrlAssociation:
        _urlAssocHash.insert(assocName, progId);
        break;

    default:
        break;
    }
}

void RegisterQAppAssociation::removeCapability(const QString &assocName)
{
    _fileAssocHash.remove(assocName);
    _urlAssocHash.remove(assocName);
}

void RegisterQAppAssociation::setAppInfo(const QString &appRegisteredName, const QString &appPath,
        const QString &appIcon, const QString &appDesc)
{
    _appRegisteredName = appRegisteredName;
    _appPath = QDir::toNativeSeparators(appPath);
    _appIcon = QDir::toNativeSeparators(appIcon);
    _appDesc = appDesc;
}

bool RegisterQAppAssociation::isPerMachineRegisteration()
{
    return (_UserRootKey == "HKEY_LOCAL_MACHINE");
}

void RegisterQAppAssociation::setPerMachineRegisteration(bool enable)
{
    if (enable) {
        _UserRootKey = "HKEY_LOCAL_MACHINE";
    }
    else {
        _UserRootKey = "HKEY_CURRENT_USER";
    }
}

bool RegisterQAppAssociation::registerAppCapabilities()
{
    if (!isVistaOrNewer()) {
        return true;
    }
    // Vista and newer
    QSettings regLocalMachine("HKEY_LOCAL_MACHINE", QSettings::NativeFormat);
    QString capabilitiesKey = regLocalMachine.value("Software/RegisteredApplications/" + _appRegisteredName).toString();

    if (capabilitiesKey.isEmpty()) {
        regLocalMachine.setValue("Software/RegisteredApplications/" + _appRegisteredName,
                                 QString("Software\\" + _appRegisteredName + "\\Capabilities"));
        capabilitiesKey = regLocalMachine.value("Software/RegisteredApplications/" + _appRegisteredName).toString();

        if (capabilitiesKey.isEmpty()) {
            QMessageBox::warning(mApp->getWindow(), tr("Warning!"),
                                 tr("There are some problems. Please, reinstall QupZilla.\n"
                                    "Maybe relaunch with administrator right do a magic for you! ;)"));
            return false;
        }
    }

    capabilitiesKey.replace("\\", "/");

    QHash<QString, QPair<QString, QString> >::const_iterator it = _assocDescHash.constBegin();
    while (it != _assocDescHash.constEnd()) {
        createProgId(it.key());
        ++it;
    }

    regLocalMachine.setValue(capabilitiesKey + "/ApplicationDescription", _appDesc);
    regLocalMachine.setValue(capabilitiesKey + "/ApplicationIcon", _appIcon);
    regLocalMachine.setValue(capabilitiesKey + "/ApplicationName", _appRegisteredName);

    QHash<QString, QString>::const_iterator i = _fileAssocHash.constBegin();
    while (i != _fileAssocHash.constEnd()) {
        regLocalMachine.setValue(capabilitiesKey + "/FileAssociations/" + i.key(), i.value());
        ++i;
    }

    i = _urlAssocHash.constBegin();
    while (i != _urlAssocHash.constEnd()) {
        regLocalMachine.setValue(capabilitiesKey + "/URLAssociations/" + i.key(), i.value());
        ++i;
    }
    regLocalMachine.setValue(capabilitiesKey + "/Startmenu/StartMenuInternet", _appPath);

    return true;
}

bool RegisterQAppAssociation::isVistaOrNewer()
{
    return (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA &&
            QSysInfo::windowsVersion() <= QSysInfo::WV_NT_based);
}

void RegisterQAppAssociation::registerAssociation(const QString &assocName, AssociationType type)
{
    if (isVistaOrNewer()) { // Vista and newer
#ifndef __MINGW32__
        IApplicationAssociationRegistration* pAAR;

        HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
                                      NULL,
                                      CLSCTX_INPROC,
                                      __uuidof(IApplicationAssociationRegistration),
                                      (void**)&pAAR);
        if (SUCCEEDED(hr)) {
            switch (type) {
            case FileAssociation:
                hr = pAAR->SetAppAsDefault(_appRegisteredName.toStdWString().c_str(),
                                           assocName.toStdWString().c_str(),
                                           AT_FILEEXTENSION);
                break;
            case UrlAssociation: {
                QSettings regCurrentUserRoot("HKEY_CURRENT_USER", QSettings::NativeFormat);
                QString currentUrlDefault =
                    regCurrentUserRoot.value("Software/Microsoft/Windows/Shell/Associations/UrlAssociations/"
                                             + assocName + "/UserChoice/Progid").toString();
                hr = pAAR->SetAppAsDefault(_appRegisteredName.toStdWString().c_str(),
                                           assocName.toStdWString().c_str(),
                                           AT_URLPROTOCOL);
                if (SUCCEEDED(hr) &&
                    !currentUrlDefault.isEmpty() &&
                    currentUrlDefault != _urlAssocHash.value(assocName)
                   ) {
                    regCurrentUserRoot.setValue("Software/Classes"
                                                + assocName
                                                + "/shell/open/command/backup_progid", currentUrlDefault);
                }
            }
            break;

            default:
                break;
            }

            pAAR->Release();
        }
#endif // #ifndef __MINGW32__
    }
    else { // Older than Vista
        QSettings regUserRoot(_UserRootKey, QSettings::NativeFormat);
        regUserRoot.beginGroup("Software/Classes");
        QSettings regClassesRoot("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
        switch (type) {
        case FileAssociation: {
            QString progId = _fileAssocHash.value(assocName);
            createProgId(progId);
            QString currentDefault = regClassesRoot.value(assocName + "/Default").toString();
            if (!currentDefault.isEmpty() &&
                currentDefault != progId &&
                regUserRoot.value(assocName + "/backup_val").toString() != progId
               ) {
                regUserRoot.setValue(assocName + "/backup_val", currentDefault);
            }
            regUserRoot.setValue(assocName + "/.", progId);
        }
        break;
        case UrlAssociation: {
            QString progId = _urlAssocHash.value(assocName);
            createProgId(progId);
            QString currentDefault = regClassesRoot.value(assocName + "/shell/open/command/Default").toString();
            QString command = "\"" + _appPath + "\" \"%1\"";
            if (!currentDefault.isEmpty() &&
                currentDefault != command &&
                regUserRoot.value(assocName + "/shell/open/command/backup_val").toString() != command
               ) {
                regUserRoot.setValue(assocName + "/shell/open/command/backup_val", currentDefault);
            }

            regUserRoot.setValue(assocName + "/shell/open/command/.", command);
            regUserRoot.setValue(assocName + "/URL Protocol", "");
            break;
        }
        default:
            break;
        }
        regUserRoot.endGroup();
    }
}

void RegisterQAppAssociation::registerAllAssociation()
{
    if (isVistaOrNewer() && !registerAppCapabilities()) {
        return;
    }

    QHash<QString, QString>::const_iterator i = _fileAssocHash.constBegin();
    while (i != _fileAssocHash.constEnd()) {
        registerAssociation(i.key(), FileAssociation);
        ++i;
    }

    i = _urlAssocHash.constBegin();
    while (i != _urlAssocHash.constEnd()) {
        registerAssociation(i.key(), UrlAssociation);
        ++i;
    }

    if (!isVistaOrNewer()) {
#ifndef __MINGW32__
        // On Windows Vista or newer for updating icons 'pAAR->SetAppAsDefault()'
        // calls 'SHChangeNotify()'. Thus, we just need care about older Windows.
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSHNOWAIT, 0 , 0);
#endif
    }
}

void RegisterQAppAssociation::createProgId(const QString &progId)
{
    QSettings regUserRoot(_UserRootKey, QSettings::NativeFormat);
    regUserRoot.beginGroup("Software/Classes");
    QPair<QString, QString> pair = _assocDescHash.value(progId);
    regUserRoot.setValue(progId + "/.", pair.first);
    regUserRoot.setValue(progId + "/shell/.", "open");
    regUserRoot.setValue(progId + "/DefaultIcon/.", pair.second);
    regUserRoot.setValue(progId + "/shell/open/command/.", QString("\"" + _appPath + "\" \"%1\""));
    regUserRoot.endGroup();
}

bool RegisterQAppAssociation::isDefaultApp(const QString &assocName, AssociationType type)
{
    if (isVistaOrNewer()) {
        QSettings regCurrentUserRoot("HKEY_CURRENT_USER", QSettings::NativeFormat);
        switch (type) {
        case FileAssociation: {
            regCurrentUserRoot.beginGroup("Software/Microsoft/Windows/CurrentVersion/Explorer/FileExts");
            if (regCurrentUserRoot.childGroups().contains(assocName, Qt::CaseInsensitive)) {
                return (_fileAssocHash.value(assocName)
                        == regCurrentUserRoot.value(assocName + "/UserChoice/Progid"));
            }
            else {
                regCurrentUserRoot.endGroup();
                return false;
            }
            break;
        }
        case UrlAssociation: {
            regCurrentUserRoot.beginGroup("Software/Microsoft/Windows/Shell/Associations/UrlAssociations");
            if (regCurrentUserRoot.childGroups().contains(assocName, Qt::CaseInsensitive)) {
                return (_urlAssocHash.value(assocName)
                        == regCurrentUserRoot.value(assocName + "/UserChoice/Progid"));
            }
            else {
                regCurrentUserRoot.endGroup();
                return false;
            }
        }
        break;

        default:
            break;
        }
    }
    else {
        QSettings regClassesRoot("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
        {
            if (!regClassesRoot.childGroups().contains(assocName, Qt::CaseInsensitive)) {
                return false;
            }
        }
        switch (type) {
        case FileAssociation: {
            return (_fileAssocHash.value(assocName)
                    == regClassesRoot.value(assocName + "/Default"));
        }
        break;
        case UrlAssociation: {
            QString currentDefault = regClassesRoot.value(assocName + "/shell/open/command/Default").toString();
            currentDefault.remove("\"");
            currentDefault.remove("%1");
            currentDefault = currentDefault.trimmed();
            return (_appPath == currentDefault);
        }
        break;

        default:
            break;
        }
    }

    return false;
}

bool RegisterQAppAssociation::isDefaultForAllCapabilities()
{
    bool result = true;
    QHash<QString, QString>::const_iterator i = _fileAssocHash.constBegin();
    while (i != _fileAssocHash.constEnd()) {
        bool res = isDefaultApp(i.key(), FileAssociation);
        result &= res;
        ++i;
    }

    i = _urlAssocHash.constBegin();
    while (i != _urlAssocHash.constEnd()) {
        bool res = isDefaultApp(i.key(), UrlAssociation);
        result &= res;
        ++i;
    }
    return result;
}
