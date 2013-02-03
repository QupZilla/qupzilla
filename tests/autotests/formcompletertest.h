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
#ifndef FORMPAGECOMPLETERTEST_H
#define FORMPAGECOMPLETERTEST_H

#include <QObject>
#include <QtTest/QtTest>

class QWebView;
struct PageFormData;

class FormCompleterTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();

    void completePageTest1();
    void completePageTest2();
    void completePageTest3();

    void extractFormTest1();
    void extractFormTest2();
    void extractFormTest3();
    void extractFormTest4();

private:
    void completeWithData(const QString &html, const QByteArray &data);
    PageFormData extractFormData(const QString &html, const QByteArray &data);
    QVariant getElementByIdValue(const QString &id);

    QWebView *view;

};

#endif // FORMPAGECOMPLETERTEST_H
