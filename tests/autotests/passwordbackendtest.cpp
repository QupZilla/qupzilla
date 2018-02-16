/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2013-2017 David Rosca <nowrep@gmail.com>
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
#include "passwordbackendtest.h"
#include "aesinterface.h"

#include <QtTest/QtTest>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>

#ifdef Q_OS_WIN
#include "qt_windows.h"
#else
#include "unistd.h"
#endif

static bool compareEntries(const PasswordEntry &value, const PasswordEntry &ref)
{
    if (ref.host != value.host) {
        qDebug() << "Host mismatch. Value =" << value.host << "Reference =" << ref.host;
        return false;
    }

    if (ref.username != value.username) {
        qDebug() << "Username mismatch. Value =" << value.username << "Reference =" << ref.username;
        return false;
    }

    if (ref.password != value.password) {
        qDebug() << "Password mismatch. Value =" << value.password << "Reference =" << ref.password;
        return false;
    }

    if (ref.data != value.data) {
        qDebug() << "Data mismatch. Value =" << value.data << "Reference =" << ref.data;
        return false;
    }

    return true;
}

PasswordBackendTest::PasswordBackendTest()
    : QObject()
    , m_backend(0)
{
}

void PasswordBackendTest::initTestCase()
{
    init();

    // Backup entries
    reloadBackend();
    m_entries = m_backend->getAllEntries();
    m_backend->removeAll();
}

void PasswordBackendTest::cleanupTestCase()
{
    cleanup();

    reloadBackend();
    foreach (const PasswordEntry &entry, m_entries) {
        m_backend->addEntry(entry);
    }
}

void PasswordBackendTest::storeTest()
{
    reloadBackend();

    /* Basic password entry */
    PasswordEntry entry;
    entry.host = "org.qupzilla.google.com";
    entry.username = "user1";
    entry.password = "pass1";
    entry.data = "entry1-data=23&username=user1&password=pass1";

    m_backend->addEntry(entry);

    // Check entry that may be stored in cache
    PasswordEntry stored = m_backend->getEntries(QUrl("org.qupzilla.google.com")).first();
    QVERIFY(compareEntries(stored, entry) == true);

    reloadBackend();

    // Check entry retrieved from backend engine
    QVERIFY(!m_backend->getEntries(QUrl("org.qupzilla.google.com")).isEmpty());
    stored = m_backend->getEntries(QUrl("org.qupzilla.google.com")).first();
    QVERIFY(compareEntries(stored, entry) == true);


    /* UTF-8 password entry */
    PasswordEntry entry2;
    entry2.host = "org.qupzilla.qupzilla.com";
    entry2.username = QString::fromUtf8("+ě ++ éí§`]|~đ11 +!:");
    entry2.password = QString::fromUtf8("+ěš asn~đ°#&# |€");
    entry2.data = "use%C2%B6+_nam%C4%8D=%2B%C4%9B+%2B%2B+%C3%A9%C3%AD%C2%A7%60%5D%7C%7E%C4%9111+%2B%21%3A"
            "&pA+%5DsQ+%2Bword=%2B%C4%9B%C5%A1+asn%7E%C4%91%C2%B0%23%26%23+%7C%E2%82%AC";

    m_backend->addEntry(entry2);

    // Check entry that may be stored in cache
    PasswordEntry stored2 = m_backend->getEntries(QUrl("org.qupzilla.qupzilla.com")).first();
    QVERIFY(compareEntries(stored2, entry2) == true);

    reloadBackend();

    // Check entry retrieved from backend engine
    stored2 = m_backend->getEntries(QUrl("org.qupzilla.qupzilla.com")).first();
    QVERIFY(compareEntries(stored2, entry2) == true);

    /* Cleanup */
    // Local cleanup
    m_backend->removeEntry(stored);
    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.google.com")).count(), 0);

    m_backend->removeEntry(stored2);
    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.qupzilla.com")).count(), 0);

    reloadBackend();

    // Backend engine cleanup
    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.google.com")).count(), 0);
    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.qupzilla.com")).count(), 0);
}

void PasswordBackendTest::removeAllTest()
{
    reloadBackend();

    PasswordEntry entry;
    entry.host = "org.qupzilla.google.com";
    entry.username = "user1";
    entry.password = "pass1";
    entry.data = "entry1-data=23&username=user1&password=pass1";
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    entry.username.append("s");
    m_backend->addEntry(entry);

    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.google.com")).count(), 7);
    reloadBackend();
    QCOMPARE(m_backend->getEntries(QUrl("org.qupzilla.google.com")).count(), 7);

    m_backend->removeAll();

    QCOMPARE(m_backend->getAllEntries().count(), 0);
    reloadBackend();
    QCOMPARE(m_backend->getAllEntries().count(), 0);
}

void PasswordBackendTest::updateLastUsedTest()
{
    reloadBackend();

    PasswordEntry entry;
    entry.host = "org.qupzilla.google.com";
    entry.username = "user1";
    entry.password = "pass1";
    entry.data = "entry1-data=23&username=user1&password=pass1";
    m_backend->addEntry(entry);

#ifdef Q_OS_WIN
    Sleep(1000);
#else
    sleep(1);
#endif

    entry.username.append("s");
    m_backend->addEntry(entry);

    QVERIFY(!m_backend->getEntries(QUrl("org.qupzilla.google.com")).isEmpty());
    QVERIFY(compareEntries(entry, m_backend->getEntries(QUrl("org.qupzilla.google.com")).first()));
    reloadBackend();
    QVERIFY(!m_backend->getEntries(QUrl("org.qupzilla.google.com")).isEmpty());
    QVERIFY(compareEntries(entry, m_backend->getEntries(QUrl("org.qupzilla.google.com")).first()));

    m_backend->removeEntry(m_backend->getEntries(QUrl("org.qupzilla.google.com")).first());
    m_backend->removeEntry(m_backend->getEntries(QUrl("org.qupzilla.google.com")).first());

    QCOMPARE(m_backend->getAllEntries().count(), 0);
    reloadBackend();
    QCOMPARE(m_backend->getAllEntries().count(), 0);
}


// DatabasePasswordBackendTest
void DatabasePasswordBackendTest::reloadBackend()
{
    delete m_backend;
    m_backend = new DatabasePasswordBackend;
}

void DatabasePasswordBackendTest::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-connection1");
    db.setDatabaseName(":memory:");
    db.open();

    db.exec("CREATE TABLE autofill (data TEXT, id INTEGER PRIMARY KEY, password TEXT,"
            "server TEXT, username TEXT, last_used NUMERIC)");
}

void DatabasePasswordBackendTest::cleanup()
{
    QSqlDatabase::removeDatabase(QSqlDatabase::database().databaseName());
}

// DatabaseEncryptedPasswordBackendTest
void DatabaseEncryptedPasswordBackendTest::reloadBackend()
{
    delete m_backend;
    DatabaseEncryptedPasswordBackend* backend = new DatabaseEncryptedPasswordBackend;

    if (m_testMasterPassword.isEmpty()) {
        m_testMasterPassword = AesInterface::passwordToHash(QString::fromUtf8(AesInterface::createRandomData(8)));
        backend->updateSampleData(m_testMasterPassword);
    }

    // a trick for setting masterPassword without gui interactions
    backend->isPasswordVerified(m_testMasterPassword);
    backend->setAskMasterPasswordState(false);

    m_backend = backend;
}

void DatabaseEncryptedPasswordBackendTest::init()
{
    QSqlDatabase db = QSqlDatabase::database("QSQLITE", "test-connection2");
    db.setDatabaseName(":memory:");
    db.open();
}

void DatabaseEncryptedPasswordBackendTest::cleanup()
{
    QSqlDatabase::removeDatabase(QSqlDatabase::database().databaseName());
}
