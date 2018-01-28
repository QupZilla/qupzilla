/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2013-2018  David Rosca <nowrep@gmail.com>
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
#include "databaseencryptedpasswordbackend.h"
#include "mainapplication.h"
#include "autofill.h"
#include "aesinterface.h"
#include "browserwindow.h"
#include "ui_masterpassworddialog.h"
#include "sqldatabase.h"

#include <QVector>
#include <QMessageBox>

#define INTERNAL_SERVER_ID QLatin1String("qupzilla.internal")

DatabaseEncryptedPasswordBackend::DatabaseEncryptedPasswordBackend()
    : PasswordBackend()
    , m_stateOfMasterPassword(UnKnownState)
    , m_askPasswordDialogVisible(false)
    , m_askMasterPassword(false)
{
    QSqlDatabase db = SqlDatabase::instance()->database();
    if (!db.tables().contains(QLatin1String("autofill_encrypted"))) {
        db.exec("CREATE TABLE autofill_encrypted (data_encrypted TEXT, id INTEGER PRIMARY KEY,"
                "password_encrypted TEXT, server TEXT, username_encrypted TEXT, last_used NUMERIC)");
        db.exec("CREATE INDEX autofillEncryptedServer ON autofill_encrypted(server ASC)");
    }
}

DatabaseEncryptedPasswordBackend::~DatabaseEncryptedPasswordBackend()
{
}

QStringList DatabaseEncryptedPasswordBackend::getUsernames(const QUrl &url)
{
    if (!m_askMasterPassword) {
        return PasswordBackend::getUsernames(url);
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT username_encrypted FROM autofill_encrypted WHERE server=? ORDER BY last_used DESC");
    query.addBindValue(PasswordManager::createHost(url));
    query.exec();

    QStringList list;
    while (query.next()) {
        list.append(QSL("Encrypted %1").arg(list.size() + 1));
    }
    return list;
}

QVector<PasswordEntry> DatabaseEncryptedPasswordBackend::getEntries(const QUrl &url)
{
    QVector<PasswordEntry> list;

    AesInterface aesDecryptor;

    const QString host = PasswordManager::createHost(url);

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, username_encrypted, password_encrypted, data_encrypted FROM autofill_encrypted "
                  "WHERE server=? ORDER BY last_used DESC");
    query.addBindValue(host);
    query.exec();

    if (query.next() && hasPermission()) {
        do {
            PasswordEntry data;
            data.id = query.value(0);
            data.host = host;
            data.username = query.value(1).toString();
            data.password = query.value(2).toString();
            data.data = query.value(3).toByteArray();

            if (decryptPasswordEntry(data, &aesDecryptor)) {
                list.append(data);
            }
        }
        while (query.next());
    }

    return list;
}

QVector<PasswordEntry> DatabaseEncryptedPasswordBackend::getAllEntries()
{
    QVector<PasswordEntry> list;

    AesInterface aesDecryptor;

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, server, username_encrypted, password_encrypted, data_encrypted FROM autofill_encrypted");
    query.exec();

    if (query.next() && hasPermission()) {
        do {
            PasswordEntry data;
            data.id = query.value(0);
            data.host = query.value(1).toString();
            if (data.host == INTERNAL_SERVER_ID) {
                continue;
            }
            data.username = query.value(2).toString();
            data.password = query.value(3).toString();
            data.data = query.value(4).toByteArray();

            if (decryptPasswordEntry(data, &aesDecryptor)) {
                list.append(data);
            }
        }
        while (query.next());
    }

    return list;
}

void DatabaseEncryptedPasswordBackend::setActive(bool active)
{
    if (active == isActive()) {
        return;
    }

    PasswordBackend::setActive(active);

    if (active) {
        setAskMasterPasswordState(isMasterPasswordSetted());
        if (!isMasterPasswordSetted()) {
            // master-password is not setted this backend needs master-password
            showMasterPasswordDialog();
        }
    }
    else {
        // maybe ask from user for decrypting data

        // remove password from memory
        m_masterPassword.clear();
        setAskMasterPasswordState(isMasterPasswordSetted());
    }
}

void DatabaseEncryptedPasswordBackend::addEntry(const PasswordEntry &entry)
{
    // Data is empty only for HTTP/FTP authorization
    if (entry.data.isEmpty()) {
        // Multiple-usernames for HTTP/FTP authorization not supported
        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("SELECT username_encrypted FROM autofill_encrypted WHERE server=?");
        query.addBindValue(entry.host);
        query.exec();

        if (query.next()) {
            return;
        }
    }

    PasswordEntry encryptedEntry = entry;
    AesInterface aesEncryptor;

    if (hasPermission() && encryptPasswordEntry(encryptedEntry, &aesEncryptor)) {
        QSqlQuery query(SqlDatabase::instance()->database());
        query.prepare("INSERT INTO autofill_encrypted (server, data_encrypted, username_encrypted, password_encrypted, last_used) "
                      "VALUES (?,?,?,?,strftime('%s', 'now'))");
        query.bindValue(0, encryptedEntry.host);
        query.bindValue(1, encryptedEntry.data);
        query.bindValue(2, encryptedEntry.username);
        query.bindValue(3, encryptedEntry.password);
        query.exec();
    }
}

bool DatabaseEncryptedPasswordBackend::updateEntry(const PasswordEntry &entry)
{
    AesInterface aesEncryptor;
    PasswordEntry encryptedEntry = entry;

    if (hasPermission() && encryptPasswordEntry(encryptedEntry, &aesEncryptor)) {
        QSqlQuery query(SqlDatabase::instance()->database());

        // Data is empty only for HTTP/FTP authorization
        if (entry.data.isEmpty()) {
            query.prepare("UPDATE autofill_encrypted SET username_encrypted=?, password_encrypted=? WHERE server=?");
            query.bindValue(0, encryptedEntry.username);
            query.bindValue(1, encryptedEntry.password);
            query.bindValue(2, encryptedEntry.host);
        }
        else {
            query.prepare("UPDATE autofill_encrypted SET data_encrypted=?, username_encrypted=?, password_encrypted=? WHERE id=?");
            query.addBindValue(encryptedEntry.data);
            query.addBindValue(encryptedEntry.username);
            query.addBindValue(encryptedEntry.password);
            query.addBindValue(encryptedEntry.id);
        }

        return query.exec();
    }

    return false;
}

void DatabaseEncryptedPasswordBackend::updateLastUsed(PasswordEntry &entry)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("UPDATE autofill_encrypted SET last_used=strftime('%s', 'now') WHERE id=?");
    query.addBindValue(entry.id);
    query.exec();
}

void DatabaseEncryptedPasswordBackend::removeEntry(const PasswordEntry &entry)
{
    if (!hasPermission()) {
        return;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("DELETE FROM autofill_encrypted WHERE id=?");
    query.addBindValue(entry.id);
    query.exec();

    m_stateOfMasterPassword = UnKnownState;
    if (someDataFromDatabase().isEmpty()) {
        updateSampleData(m_masterPassword);
    }
}

void DatabaseEncryptedPasswordBackend::removeAll()
{
    if (!hasPermission()) {
        return;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("DELETE FROM autofill_encrypted");
    query.exec();

    m_stateOfMasterPassword = PasswordIsSetted;

    updateSampleData(m_masterPassword);
}

QString DatabaseEncryptedPasswordBackend::name() const
{
    return AutoFill::tr("Database (encrypted)");
}

bool DatabaseEncryptedPasswordBackend::hasSettings() const
{
    return true;
}

void DatabaseEncryptedPasswordBackend::showSettings(QWidget* parent)
{
    MasterPasswordDialog* masterPasswordDialog = new MasterPasswordDialog(this, parent);
    masterPasswordDialog->showSettingPage();
}

bool DatabaseEncryptedPasswordBackend::isMasterPasswordSetted()
{
    if (m_stateOfMasterPassword == UnKnownState) {
        m_stateOfMasterPassword = someDataFromDatabase().isEmpty() ? PasswordIsNotSetted : PasswordIsSetted;
    }

    return m_stateOfMasterPassword == PasswordIsSetted;
}

QByteArray DatabaseEncryptedPasswordBackend::masterPassword() const
{
    return m_masterPassword;
}

bool DatabaseEncryptedPasswordBackend::hasPermission()
{
    if (!m_askMasterPassword) {
        return true;
    }

    if (m_askPasswordDialogVisible) {
        return false;
    }

    m_askPasswordDialogVisible = true;

    AskMasterPassword* dialog = new AskMasterPassword(this);

    bool authorized = dialog->exec() == QDialog::Accepted;

    m_askPasswordDialogVisible = false;
    return authorized;
}

bool DatabaseEncryptedPasswordBackend::isPasswordVerified(const QByteArray &password)
{
    if (password.isEmpty()) {
        return false;
    }

    if (m_masterPassword == password) {
        return true;
    }
    else if (!m_masterPassword.isEmpty()) {
        return false;
    }
    else {
        // m_masterPassword is empty we need to check entered password with
        // decoding some data by it and then save it to m_masterPassword
        AesInterface aes;
        aes.decrypt(someDataFromDatabase(), password);
        if (aes.isOk()) {
            m_masterPassword = password;
            return true;
        }
    }

    return false;
}

bool DatabaseEncryptedPasswordBackend::decryptPasswordEntry(PasswordEntry &entry, AesInterface* aesInterface)
{
    entry.username = QString::fromUtf8(aesInterface->decrypt(entry.username.toUtf8(), m_masterPassword));
    entry.password = QString::fromUtf8(aesInterface->decrypt(entry.password.toUtf8(), m_masterPassword));
    entry.data = aesInterface->decrypt(entry.data, m_masterPassword);

    return aesInterface->isOk();
}

bool DatabaseEncryptedPasswordBackend::encryptPasswordEntry(PasswordEntry &entry, AesInterface* aesInterface)
{
    entry.username = QString::fromUtf8(aesInterface->encrypt(entry.username.toUtf8(), m_masterPassword));
    entry.password = QString::fromUtf8(aesInterface->encrypt(entry.password.toUtf8(), m_masterPassword));
    entry.data = aesInterface->encrypt(entry.data, m_masterPassword);

    return aesInterface->isOk();
}

void DatabaseEncryptedPasswordBackend::showMasterPasswordDialog()
{
    MasterPasswordDialog* masterPasswordDialog = new MasterPasswordDialog(this, mApp->getWindow());
    masterPasswordDialog->showSetMasterPasswordPage();
    masterPasswordDialog->delayedExec();
}

void DatabaseEncryptedPasswordBackend::tryToChangeMasterPassword(const QByteArray &newPassword)
{
    if (m_masterPassword == newPassword) {
        return;
    }

    if (newPassword.isEmpty()) {
        removeMasterPassword();
        return;
    }

    encryptDataBaseTableOnFly(m_masterPassword, newPassword);

    m_masterPassword = newPassword;
    updateSampleData(m_masterPassword);
}

void DatabaseEncryptedPasswordBackend::removeMasterPassword()
{
    if (!m_masterPassword.isEmpty()) {
        encryptDataBaseTableOnFly(m_masterPassword, QByteArray());

        m_masterPassword.clear();
        updateSampleData(QByteArray());
    }
}

void DatabaseEncryptedPasswordBackend::setAskMasterPasswordState(bool ask)
{
    m_askMasterPassword = ask;
}

void DatabaseEncryptedPasswordBackend::encryptDataBaseTableOnFly(const QByteArray &decryptorPassword, const QByteArray &encryptorPassword)
{
    if (encryptorPassword == decryptorPassword) {
        return;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id, data_encrypted, password_encrypted, username_encrypted, server FROM autofill_encrypted");
    query.exec();

    AesInterface encryptor;
    AesInterface decryptor;

    while (query.next()) {
        QString server = query.value(4).toString();
        if (server == INTERNAL_SERVER_ID) {
            continue;
        }

        int id = query.value(0).toInt();
        QByteArray data = query.value(1).toString().toUtf8();
        QByteArray password = query.value(2).toString().toUtf8();
        QByteArray username = query.value(3).toString().toUtf8();

        if (!decryptorPassword.isEmpty()) {
            data = decryptor.decrypt(data, decryptorPassword);
            password = decryptor.decrypt(password, decryptorPassword);
            username = decryptor.decrypt(username, decryptorPassword);
        }

        if (!encryptorPassword.isEmpty()) {
            data = encryptor.encrypt(data, encryptorPassword);
            password = encryptor.encrypt(password, encryptorPassword);
            username = encryptor.encrypt(username, encryptorPassword);
        }

        QSqlQuery updateQuery(SqlDatabase::instance()->database());
        updateQuery.prepare("UPDATE autofill_encrypted SET data_encrypted = ?, password_encrypted = ?, username_encrypted = ? WHERE id = ?");
        updateQuery.addBindValue(data);
        updateQuery.addBindValue(password);
        updateQuery.addBindValue(username);
        updateQuery.addBindValue(id);
        query.exec();
    }
}

QByteArray DatabaseEncryptedPasswordBackend::someDataFromDatabase()
{
    if (m_stateOfMasterPassword != UnKnownState && !m_someDataStoredOnDataBase.isEmpty()) {
        return m_someDataStoredOnDataBase;
    }

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT password_encrypted, data_encrypted, username_encrypted FROM autofill_encrypted");
    query.exec();

    QByteArray someData;
    if (query.next()) {
        int i = 0;
        while (someData.isEmpty()) {
            if (i > 2) {
                if (query.next()) {
                    i = 0;
                    continue;
                }
                else {
                    break;
                }
            }
            someData = query.value(i).toByteArray();
            ++i;
        }
    }

    m_someDataStoredOnDataBase = someData;
    return m_someDataStoredOnDataBase;
}

void DatabaseEncryptedPasswordBackend::updateSampleData(const QByteArray &password)
{
    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare("SELECT id FROM autofill_encrypted WHERE server = ?");
    query.addBindValue(INTERNAL_SERVER_ID);
    query.exec();

    if (!password.isEmpty()) {
        AesInterface aes;
        m_someDataStoredOnDataBase = aes.encrypt(AesInterface::createRandomData(16), password);

        if (query.next()) {
            query.prepare("UPDATE autofill_encrypted SET password_encrypted = ? WHERE server=?");
        }
        else {
            query.prepare("INSERT INTO autofill_encrypted (password_encrypted, server) VALUES (?,?)");
        }

        query.addBindValue(QString::fromUtf8(m_someDataStoredOnDataBase));
        query.addBindValue(INTERNAL_SERVER_ID);
        query.exec();

        m_stateOfMasterPassword = PasswordIsSetted;
    }
    else if (query.next()) {
        query.prepare("DELETE FROM autofill_encrypted WHERE server = ?");
        query.addBindValue(INTERNAL_SERVER_ID);
        query.exec();

        m_stateOfMasterPassword = PasswordIsNotSetted;
        m_someDataStoredOnDataBase.clear();
        return;
    }
}


/******************************
 * MasterPasswordDialog class *
 ******************************/

#include <QTimer>

MasterPasswordDialog::MasterPasswordDialog(DatabaseEncryptedPasswordBackend* backend, QWidget* parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint)
    , ui(new Ui::MasterPasswordDialog)
    , m_backend(backend)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->setupUi(this);

    ui->currentPassword->setVisible(m_backend->isMasterPasswordSetted());
    ui->labelCurrentPassword->setVisible(m_backend->isMasterPasswordSetted());

    connect(ui->setMasterPassword, SIGNAL(clicked()), this, SLOT(showSetMasterPasswordPage()));
    connect(ui->clearMasterPassword, SIGNAL(clicked()), this, SLOT(clearMasterPasswordAndConvert()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->buttonBoxMasterPassword, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->buttonBoxMasterPassword, SIGNAL(accepted()), this, SLOT(accept()));
}

MasterPasswordDialog::~MasterPasswordDialog()
{
    delete ui;
}

void MasterPasswordDialog::delayedExec()
{
    QTimer::singleShot(0, this, SLOT(exec()));
}

void MasterPasswordDialog::accept()
{
    if (ui->stackedWidget->currentIndex() != 1) {
        QDialog::accept();
        return;
    }

    QByteArray currentPassField = AesInterface::passwordToHash(ui->currentPassword->text());

    if (m_backend->isMasterPasswordSetted() && !m_backend->isPasswordVerified(currentPassField)) {
        QMessageBox::information(this, tr("Warning!"), tr("You entered a wrong password!"));
        return;
    }

    if (ui->newPassword->text() != ui->confirmPassword->text()) {
        QMessageBox::information(this, tr("Warning!"), tr("New/Confirm password fields do not match!"));
        return;
    }

    if (ui->newPassword->text().isEmpty()) {
        if (!m_backend->isMasterPasswordSetted()) {
            return;
        }
        clearMasterPasswordAndConvert(false);
    }
    else {
        // for security reason we don't save master-password as plain in memory
        QByteArray newPassField = AesInterface::passwordToHash(ui->newPassword->text());

        if (m_backend->masterPassword() != newPassField) {
            m_backend->tryToChangeMasterPassword(newPassField);
        }
    }
    QDialog::accept();
}

void MasterPasswordDialog::reject()
{
    QDialog::reject();

    if (m_backend->isActive() && !m_backend->isMasterPasswordSetted()) {
        // master password not setted
        QMessageBox::information(this, AutoFill::tr("Warning!"),
                                 AutoFill::tr("This backend needs a master password to be set! "
                                              "QupZilla just switches to its default backend"));
        // active default backend
        mApp->autoFill()->passwordManager()->switchBackend("database");
        return;
    }
}

void MasterPasswordDialog::showSettingPage()
{
    ui->stackedWidget->setCurrentIndex(0);
    delayedExec();
}

void MasterPasswordDialog::showSetMasterPasswordPage()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MasterPasswordDialog::clearMasterPasswordAndConvert(bool forcedAskPass)
{
    if (QMessageBox::information(this, tr("Warning!"), tr("Are you sure you want to clear master password and decrypt data?"), QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::No) {
        reject();
        return;
    }

    if (forcedAskPass) {
        m_backend->setAskMasterPasswordState(true);
    }

    if (m_backend->hasPermission()) {
        QVector<PasswordEntry> list = m_backend->getAllEntries();
        PasswordBackend* databaseBackend = mApp->autoFill()->passwordManager()->availableBackends().value("database");
        if (!databaseBackend) {
            return;
        }

        QVector<PasswordEntry> databaseList = databaseBackend->getAllEntries();
        bool allDataMoved = true;
        foreach (const PasswordEntry &entry, list) {
            bool sameEntry = false;
            foreach (const PasswordEntry &dbEntry, databaseList) {
                sameEntry = samePasswordEntry(dbEntry, entry);
                if (sameEntry) {
                    allDataMoved = false;
                    break;
                }
            }

            if (!sameEntry) {
                databaseBackend->addEntry(entry);
                m_backend->removeEntry(entry);
            }
        }

        if (allDataMoved) {
            m_backend->removeAll();
            m_backend->removeMasterPassword();
            m_backend->setAskMasterPasswordState(false);

            mApp->autoFill()->passwordManager()->switchBackend("database");
        }
        else {
            QMessageBox::information(this, tr("Warning!"), tr("Some data has not been decrypted. The master password was not cleared!"));
            mApp->autoFill()->passwordManager()->switchBackend("database");
        }
    }
    reject();
}

bool MasterPasswordDialog::samePasswordEntry(const PasswordEntry &entry1, const PasswordEntry &entry2)
{
    // Multiple-usernames for HTTP/FTP authorization not supported
    if ((entry1.data.isEmpty() || entry2.data.isEmpty()) && entry1.host == entry2.host) {
        return true;
    }

    if (entry1.host != entry2.host || entry1.username != entry2.username) {
        return false;
    }
    return true;
}


AskMasterPassword::AskMasterPassword(DatabaseEncryptedPasswordBackend* backend, QWidget* parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint)
    , m_backend(backend)
{
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(AutoFill::tr("Enter Master Password"));

    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    QLabel* label = new QLabel(this);
    label->setText(AutoFill::tr("Permission is required, please enter Master Password:"));
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    m_labelWarning = new QLabel(this);
    m_labelWarning->setText(AutoFill::tr("Entered password is wrong!"));
    QPalette pal = m_labelWarning->palette();
    pal.setBrush(QPalette::WindowText, Qt::red);
    m_labelWarning->setPalette(pal);
    m_labelWarning->hide();

    verticalLayout->addWidget(label);
    verticalLayout->addWidget(m_lineEdit);
    verticalLayout->addWidget(m_labelWarning);
    verticalLayout->addWidget(m_buttonBox);
    setLayout(verticalLayout);

    connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(verifyPassword()));
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(verifyPassword()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setAttribute(Qt::WA_DeleteOnClose);
}

void AskMasterPassword::verifyPassword()
{
    QByteArray enteredPassword = AesInterface::passwordToHash(m_lineEdit->text());
    if (!m_backend->isPasswordVerified(enteredPassword)) {
        m_backend->setAskMasterPasswordState(true);
        m_labelWarning->show();
        m_lineEdit->clear();
        m_lineEdit->setFocus();
    }
    else {
        m_backend->setAskMasterPasswordState(false);
        //TODO: start timer for reset ask state to true

        accept();
    }
}
