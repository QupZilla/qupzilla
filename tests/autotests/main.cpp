/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "qztoolstest.h"
#include "formcompletertest.h"
#include "cookiestest.h"
#include "downloadstest.h"
#include "adblocktest.h"
#include "updatertest.h"
#include "pactest.h"
#include "passwordbackendtest.h"

#include <QtTest/QtTest>

#define RUN_TEST(X) \
    { \
    qDebug() << ""; \
    X t; \
    int r = QTest::qExec(&t, argc, argv); \
    if (r != 0) return 1; \
    }

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTEST_DISABLE_KEYPAD_NAVIGATION;

    RUN_TEST(QzToolsTest)
    RUN_TEST(FormCompleterTest)
    RUN_TEST(CookiesTest)
    RUN_TEST(DownloadsTest)
    RUN_TEST(AdBlockTest)
    RUN_TEST(UpdaterTest)
    RUN_TEST(PacTest)

    RUN_TEST(DatabasePasswordBackendTest)
    RUN_TEST(KWalletPasswordBackendTest)
    RUN_TEST(GnomeKeyringPasswordBackendTest)

    return 0;
}
