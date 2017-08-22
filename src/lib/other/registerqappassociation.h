/* ============================================================
* Copyright (C) 2012-2017  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef REGISTERQAPPASSOCIATION_H
#define REGISTERQAPPASSOCIATION_H

#include <QObject>
#include <QHash>
#include <QPair>

#include "qzcommon.h"

class QUPZILLA_EXPORT RegisterQAppAssociation : public QObject
{
    Q_OBJECT
public:
    explicit RegisterQAppAssociation(QObject* parent = 0);
    explicit RegisterQAppAssociation(const QString &appRegisteredName, const QString &appPath,
                                     const QString &appIcon = QString(), const QString &appDesc = QString(), QObject* parent = 0);
    ~RegisterQAppAssociation();

    enum AssociationType {
        FileAssociation,
        UrlAssociation
    };

    void addCapability(const QString &assocName, const QString &progId,
                       const QString &desc, const QString &iconPath, AssociationType type);
    void removeCapability(const QString &assocName);

    void setAppInfo(const QString &appRegisteredName, const QString &appPath,
                    const QString &appIcon = QString(), const QString &appDesc = QString());

    bool isPerMachineRegisteration();
    void setPerMachineRegisteration(bool enable);
    bool registerAppCapabilities();
    bool isVistaOrNewer();
    bool isWin10OrNewer();
    void registerAssociation(const QString &assocName, AssociationType type);
    void createProgId(const QString &progId);

    bool isDefaultApp(const QString &assocName, AssociationType type);
    bool isDefaultForAllCapabilities();
    void registerAllAssociation();

    bool showNativeDefaultAppSettingsUi();

private:
    QString _appRegisteredName;
    QString _appPath;
    QString _appIcon;
    QString _appDesc;
    QString _UserRootKey;

    QHash<QString, QString> _fileAssocHash; // (extention, progId)
    QHash<QString, QString> _urlAssocHash; // (protocol, progId)
    QHash<QString, QPair<QString, QString> > _assocDescHash; // (progId, (desc, icon))
};

#endif // REGISTERQAPPASSOCIATION_H
