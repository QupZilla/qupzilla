/* ============================================================
* Copyright (C) 2012  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* This file is part of QupZilla - WebKit based browser 2010-2012
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

#include <QHash>
#include <QPair>

#include "qz_namespace.h"

class QT_QUPZILLA_EXPORT RegisterQAppAssociation : public QObject
{
    Q_OBJECT
public:
    explicit RegisterQAppAssociation(QObject* parent = 0);
    explicit RegisterQAppAssociation(const QString &appRegisteredName, const QString &appPath,
                               const QString &appIcon = "", const QString &appDesc = "", QObject* parent = 0);
    ~RegisterQAppAssociation();

    enum AssociationType {
        FileAssociation,
        UrlAssociation
    };

    void addCapability(const QString &assocName, const QString &progId,
                       const QString &desc, const QString &iconPath, AssociationType type);
    void removeCapability(const QString &assocName);

    void setAppInfo(const QString &appRegisteredName, const QString &appPath,
                    const QString &appIcon = "", const QString &appDesc = "");

    bool isPerMachineRegisteration();
    void setPerMachineRegisteration(bool enable);
#ifdef Q_OS_WIN
    bool registerAppCapabilities();
    bool isVistaOrNewer();
#endif
    void registerAssociation(const QString &assocName, AssociationType type);
    void createProgId(const QString &progId);

    bool isDefaultApp(const QString &assocName, AssociationType type);
    bool isDefaultForAllCapabilities();
    void registerAllAssociation();


private:
    QString _appRegisteredName;
    QString _appPath;
    QString _appIcon;
    QString _appDesc;
#ifdef Q_OS_WIN
    QString _UserRootKey;
#endif

    QHash<QString, QString> _fileAssocHash; // (extention, progId)
    QHash<QString, QString> _urlAssocHash; // (protocol, progId)
    QHash<QString, QPair<QString, QString> > _assocDescHash; // (progId, (desc, icon))
};

#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class QT_QUPZILLA_EXPORT CheckMessageBox : public QDialog
{
    Q_OBJECT

public:
    CheckMessageBox(bool *defaultShowAgainState = 0, QWidget *parent = 0, Qt::WindowFlags f = 0);
    CheckMessageBox(const QString &msg, const QPixmap &pixmap,
                    const QString &str, bool *defaultShowAgainState,
                    QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CheckMessageBox();

    void setMessage(const QString &msg);
    void setShowAgainText(const QString &str);
    void setPixmap(const QPixmap &pixmap);

private:
    void setupUi();

    bool *_showAgainState;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QLabel *pixmapLabel;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout;
    QLabel *messageLabel;
    QSpacerItem *horizontalSpacer;
    QCheckBox *showAgainCheckBox;
    QDialogButtonBox *buttonBox;

private slots:
    void showAgainStateChanged(bool checked);
};
#endif // REGISTERQAPPASSOCIATION_H
