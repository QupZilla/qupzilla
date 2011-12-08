/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef AUTOFILLMODEL_H
#define AUTOFILLMODEL_H

#include <QObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QWebElement>

class QupZilla;
class WebView;
class AutoFillModel : public QObject
{
    Q_OBJECT
public:
    typedef QList<QPair<QString, QString> > QueryItems;
    typedef QPair<QString, QString> QueryItem;

    explicit AutoFillModel(QupZilla* mainClass, QObject* parent = 0);
    void completePage(WebView* view);

    bool isStored(const QUrl &url);
    bool isStoringEnabled(const QUrl &url);
    void blockStoringfor(const QUrl &url);

    QString getUsername(const QUrl &url);
    QString getPassword(const QUrl &url);
    void addEntry(const QUrl &url, const QString &name, const QString &pass);
    void addEntry(const QUrl &url, const QByteArray &data, const QString &user, const QString &pass);

    void post(const QNetworkRequest &request, const QByteArray &outgoingData);

signals:

public slots:
    void loadSettings();

private:
    bool dataContains(const QByteArray &data, const QString &attributeName);
    QString getValueFromData(const QByteArray &data, QWebElement element);

    QupZilla* p_QupZilla;
    QByteArray m_lastOutgoingData;
    bool m_isStoring;

};

#endif // AUTOFILLMODEL_H
