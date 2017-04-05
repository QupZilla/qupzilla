/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2017  Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#include "browserwindow.h"
#include "datapaths.h"
#include "mainapplication.h"
#include "restoremanager.h"
#include "sessionmanager.h"
#include "settings.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileSystemWatcher>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>


SessionManager::SessionManager(QObject* parent)
    : QObject(parent)
    , m_firstBackupSession(DataPaths::currentProfilePath() + QL1S("/session.dat.old"))
    , m_secondBackupSession(DataPaths::currentProfilePath() + QL1S("/session.dat.old1"))
{
    QFileSystemWatcher* sessionFilesWatcher = new QFileSystemWatcher({DataPaths::path(DataPaths::Sessions)}, this);
    connect(sessionFilesWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(sessionsDirectoryChanged()));

    loadSettings();
}

static void addSessionSubmenu(QObject* receiver, QMenu* menu, const QString &title, const QString &filePath, const QFileInfo &lastActiveSessionFileInfo)
{
    QMenu* sessionSubmenu = new QMenu(title, menu);
    QObject::connect(sessionSubmenu, SIGNAL(aboutToShow()), receiver, SLOT(aboutToShowSessionSubmenu()));

    QAction* action = menu->addMenu(sessionSubmenu);
    action->setData(filePath);
    action->setCheckable(true);
    action->setChecked(QFileInfo(filePath) == lastActiveSessionFileInfo);
}

static void addSessionsMetaDataToMenu(QObject* receiver, QMenu* menu, const QFileInfo &lastActiveSessionFileInfo, const QList<SessionManager::SessionMetaData> &sessionsMetaDataList)
{
    for (const SessionManager::SessionMetaData &metaData : sessionsMetaDataList) {
        addSessionSubmenu(receiver, menu, metaData.name, metaData.filePath, lastActiveSessionFileInfo);
    }
}

void SessionManager::aboutToShowSessionsMenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    menu->clear();

    const QFileInfo lastActiveSessionInfo(m_lastActiveSessionPath);

    fillSessionsMetaDataListIfNeeded();

    addSessionsMetaDataToMenu(this, menu, lastActiveSessionInfo, m_sessionsMetaDataList);

    menu->addSeparator();

    if (QFile::exists(m_firstBackupSession))
        addSessionSubmenu(this, menu, tr("First Backup"), m_firstBackupSession, lastActiveSessionInfo);
    if (QFile::exists(m_secondBackupSession))
        addSessionSubmenu(this, menu, tr("Second Backup"), m_secondBackupSession, lastActiveSessionInfo);
}

void SessionManager::aboutToShowSessionSubmenu()
{
    QMenu* menu = qobject_cast<QMenu*>(sender());
    menu->clear();

    const QString sessionFilePath = menu->menuAction()->data().toString();

    const QFileInfo currentSessionFileInfo(m_lastActiveSessionPath);
    const QFileInfo sessionFileInfo(sessionFilePath);

    QList<QAction*> actions;
    QAction* action;

    if (sessionFileInfo != currentSessionFileInfo || mApp->restoreManager()) {
        if (sessionFileInfo != QFileInfo(m_firstBackupSession) && sessionFileInfo != QFileInfo(m_secondBackupSession)) {
            action = new QAction(SessionManager::tr("Switch To"), menu);
            action->setData(sessionFilePath);
            QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(switchToSession()));
            actions << action;
        }

        action = new QAction(SessionManager::tr("Open"), menu);
        action->setData(sessionFilePath);
        QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(openSession()));
        actions << action;

        action = new QAction(menu);
        action->setSeparator(true);
        actions << action;
    }

    action = new QAction(SessionManager::tr("Clone"), menu);
    action->setData(sessionFilePath);
    QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(cloneSession()));
    actions << action;

    if (sessionFileInfo != currentSessionFileInfo) {
        action = new QAction(SessionManager::tr("Rename"), menu);
        action->setData(sessionFilePath);
        QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(renameSession()));
        actions << action;
    }

    if (sessionFileInfo != currentSessionFileInfo && sessionFileInfo != QFileInfo(defaultSessionPath()) &&
            sessionFileInfo != QFileInfo(m_firstBackupSession) && sessionFileInfo != QFileInfo(m_secondBackupSession)) {
        action = new QAction(SessionManager::tr("Remove"), menu);
        action->setData(sessionFilePath);
        QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteSession()));
        actions << action;
    }

    menu->addActions(actions);
}

void SessionManager::sessionsDirectoryChanged()
{
    m_sessionsMetaDataList.clear();
}

void SessionManager::switchToSession()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    openSession(action->data().toString(), true);
}

void SessionManager::openSession(QString sessionFilePath, bool switchSession)
{
    if (sessionFilePath.isEmpty()) {
        QAction* action = qobject_cast<QAction*>(sender());
        if (!action)
            return;

        sessionFilePath = action->data().toString();
    }

    QVector<RestoreManager::WindowData> sessionData;
    RestoreManager::createFromFile(sessionFilePath, sessionData);

    if (sessionData.isEmpty())
        return;

    BrowserWindow* window = mApp->getWindow();
    if (switchSession) {
        writeCurrentSession(m_lastActiveSessionPath);

        window = mApp->createWindow(Qz::BW_OtherRestoredWindow);
        for (BrowserWindow* win : mApp->windows()) {
            if (win != window)
                win->close();
        }

        m_lastActiveSessionPath = QFileInfo(sessionFilePath).canonicalFilePath();
    }

    mApp->openSession(window, sessionData);
}

void SessionManager::renameSession(QString sessionFilePath, bool clone)
{
    if (sessionFilePath.isEmpty()) {
        QAction* action = qobject_cast<QAction*>(sender());
        if (!action)
            return;

        sessionFilePath = action->data().toString();
    }

    bool ok;
    const QString suggestedName = QFileInfo(sessionFilePath).baseName() + (clone ? tr("_cloned") : tr("_renamed"));
    QString newName = QInputDialog::getText(mApp->activeWindow(), (clone ? tr("Clone Session") : tr("Rename Session")),
                                            tr("Please enter a new name:"), QLineEdit::Normal,
                                            suggestedName, &ok);

    if (!ok)
        return;

    const QString newSessionPath = QString("%1/%2.dat").arg(DataPaths::path(DataPaths::Sessions)).arg(newName);
    if (QFile::exists(newSessionPath)) {
        QMessageBox::information(mApp->activeWindow(), tr("Error!"), tr("The session file \"%1\" exists. Please enter another name.").arg(newName));
        renameSession(sessionFilePath, clone);
        return;
    }

    if (clone) {
        if (!QFile::copy(sessionFilePath, newSessionPath)) {
            QMessageBox::information(mApp->activeWindow(), tr("Error!"), tr("An error occurred when cloning session file."));
            return;
        }
    }
    else if (!QFile::rename(sessionFilePath, newSessionPath)) {
        QMessageBox::information(mApp->activeWindow(), tr("Error!"), tr("An error occurred when renaming session file."));
        return;
    }
}

void SessionManager::cloneSession()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    renameSession(action->data().toString(), true);
}

void SessionManager::deleteSession()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    const QString filePath = action->data().toString();

    QMessageBox::StandardButton result = QMessageBox::information(mApp->activeWindow(), tr("Warning!"), tr("Are you sure to delete following session?\n%1")
                                                                  .arg(QDir::toNativeSeparators(filePath)), QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        QFile::remove(filePath);
    }
}

void SessionManager::saveSession()
{
    bool ok;
    QString sessionName = QInputDialog::getText(mApp->activeWindow(), tr("Save Session"),
                                         tr("Please enter a name to save session:"), QLineEdit::Normal,
                                         tr("Saved Session (%1)").arg(QDateTime::currentDateTime().toString("dd MMM yyyy HH-mm-ss")), &ok);

    if (!ok)
        return;

    const QString filePath = QString("%1/%2.dat").arg(DataPaths::path(DataPaths::Sessions)).arg(sessionName);
    if (QFile::exists(filePath)) {
        QMessageBox::information(mApp->activeWindow(), tr("Error!"), tr("The session file \"%1\" exists. Please enter another name.").arg(sessionName));
        saveSession();
        return;
    }

    writeCurrentSession(filePath);
}

void SessionManager::newSession()
{
    bool ok;
    QString sessionName = QInputDialog::getText(mApp->activeWindow(), tr("New Session"),
                                         tr("Please enter a name to create new session:"), QLineEdit::Normal,
                                         tr("New Session (%1)").arg(QDateTime::currentDateTime().toString("dd MMM yyyy HH-mm-ss")), &ok);

    if (!ok)
        return;

    const QString filePath = QString("%1/%2.dat").arg(DataPaths::path(DataPaths::Sessions)).arg(sessionName);
    if (QFile::exists(filePath)) {
        QMessageBox::information(mApp->activeWindow(), tr("Error!"), tr("The session file \"%1\" exists. Please enter another name.").arg(sessionName));
        newSession();
        return;
    }

    writeCurrentSession(m_lastActiveSessionPath);

    BrowserWindow* window = mApp->createWindow(Qz::BW_NewWindow);
    for (BrowserWindow* win : mApp->windows()) {
        if (win != window)
            win->close();
    }

    m_lastActiveSessionPath = filePath;
    autoSaveLastSession();
}

void SessionManager::fillSessionsMetaDataListIfNeeded()
{
    if (!m_sessionsMetaDataList.isEmpty())
        return;

    QDir dir(DataPaths::path(DataPaths::Sessions));

    const QFileInfoList sessionFiles = QFileInfoList() << QFileInfo(defaultSessionPath()) << dir.entryInfoList({QSL("*.*")}, QDir::Files, QDir::Time);

    QStringList fileNames;

    for (int i = 0; i < sessionFiles.size(); ++i) {
        const QFileInfo &fileInfo = sessionFiles.at(i);
        QVector<RestoreManager::WindowData> data;
        RestoreManager::createFromFile(fileInfo.absoluteFilePath(), data);

        if (data.isEmpty())
            continue;

        SessionMetaData metaData;
        metaData.name = fileInfo.baseName();

        if (fileInfo == QFileInfo(defaultSessionPath()))
            metaData.name = tr("Default Session");
        else if (fileNames.contains(fileInfo.baseName()))
            metaData.name = fileInfo.fileName();
        else
            metaData.name = fileInfo.baseName();

        fileNames << metaData.name;
        metaData.filePath = fileInfo.canonicalFilePath();

        m_sessionsMetaDataList << metaData;
    }
}

void SessionManager::loadSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    m_lastActiveSessionPath = settings.value("lastActiveSessionPath", defaultSessionPath()).toString();
    settings.endGroup();

    // fallback to default session
    if (!QFile::exists(m_lastActiveSessionPath))
        m_lastActiveSessionPath = defaultSessionPath();
}

void SessionManager::saveSettings()
{
    Settings settings;
    settings.beginGroup("Web-Browser-Settings");
    settings.setValue("lastActiveSessionPath", m_lastActiveSessionPath);
    settings.endGroup();
}

QString SessionManager::defaultSessionPath()
{
    return DataPaths::currentProfilePath() + QL1S("/session.dat");
}

QString SessionManager::lastActiveSessionPath() const
{
    return m_lastActiveSessionPath;
}

void SessionManager::backupSavedSessions()
{
    if (!QFile::exists(m_lastActiveSessionPath)) {
        return;
    }

    if (QFile::exists(m_firstBackupSession)) {
        QFile::remove(m_secondBackupSession);
        QFile::copy(m_firstBackupSession, m_secondBackupSession);
    }

    QFile::remove(m_firstBackupSession);
    QFile::copy(m_lastActiveSessionPath, m_firstBackupSession);
}

void SessionManager::writeCurrentSession(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) || file.write(mApp->saveState()) == -1) {
        qWarning() << "Error! can not write the current session file: " << filePath << file.errorString();
        return;
    }
    file.close();
}

void SessionManager::autoSaveLastSession()
{
    if (mApp->isPrivate() || mApp->isRestoring() || mApp->windowCount() == 0 || mApp->restoreManager()) {
        return;
    }

    saveSettings();
    writeCurrentSession(m_lastActiveSessionPath);
}

QString SessionManager::askSessionFromUser()
{
    fillSessionsMetaDataListIfNeeded();

    QDialog dialog(mApp->getWindow(), Qt::WindowStaysOnTopHint);
    QLabel label(tr("Please select the startup session:"), &dialog);
    QComboBox comboBox(&dialog);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    QVBoxLayout layout;
    layout.addWidget(&label);
    layout.addWidget(&comboBox);
    layout.addWidget(&buttonBox);
    dialog.setLayout(&layout);

    const QFileInfo lastActiveSessionFileInfo(m_lastActiveSessionPath);

    for (const SessionMetaData &metaData : m_sessionsMetaDataList) {
        if (QFileInfo(metaData.filePath) != lastActiveSessionFileInfo) {
            comboBox.addItem(metaData.name, metaData.filePath);
        }
        else {
            comboBox.insertItem(0, tr("%1 (last session)").arg(metaData.name), metaData.filePath);
        }
    }

    comboBox.setCurrentIndex(0);

    if (dialog.exec() == QDialog::Accepted) {
        m_lastActiveSessionPath = comboBox.currentData().toString();
    }

    return m_lastActiveSessionPath;
}
