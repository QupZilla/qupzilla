/* ============================================================
* Access Keys Navigation plugin for QupZilla
* Copyright (C) 2012  David Rosca <nowrep@gmail.com>
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
#ifndef AKN_HANDLER_H
#define AKN_HANDLER_H

#include <QObject>
#include <QTime>
#include <QHash>
#include <QWebElement>
#include <QWeakPointer>

class QKeyEvent;
class QWebElement;
class QTime;
class QLabel;

class WebView;

class AKN_Handler : public QObject
{
    Q_OBJECT
public:
    explicit AKN_Handler(const QString &sPath, QObject* parent = 0);

    QString settingsPath();
    void loadSettings();

    bool handleKeyPress(QObject* obj, QKeyEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

signals:

public slots:

private slots:
    void showAccessKeys();
    void hideAccessKeys();

private:
    void triggerShowAccessKeys();

    void makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element);
    void handleAccessKey(QKeyEvent* event);

    QWeakPointer<WebView> m_view;

    QList<QLabel*> m_accessKeyLabels;
    QHash<QChar, QWebElement> m_accessKeyNodes;
    bool m_accessKeysVisible;

    Qt::Key m_key;
    bool m_isDoublePress;
    QTime m_lastKeyPressTime;
    QString m_settingsPath;
};

#endif // AKN_HANDLER_H
