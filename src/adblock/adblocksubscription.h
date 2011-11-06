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
/**
 * Copyright (c) 2009, Benjamin C. Meyer <ben@meyerhome.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Benjamin Meyer nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef ADBLOCKSUBSCRIPTION_H
#define ADBLOCKSUBSCRIPTION_H

#include "adblockrule.h"
#include <QObject>
#include <QList>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QNetworkReply>
#include <QTextStream>
#include <QFileInfo>

class QNetworkReply;
class QUrl;
class AdBlockSubscription : public QObject
{
    Q_OBJECT

signals:
    void changed();
    void rulesChanged();

public:
    AdBlockSubscription(QObject* parent = 0);

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    void updateNow();
    QDateTime lastUpdate() const;

    void saveRules();

    const AdBlockRule* allow(const QString &urlString) const;
    const AdBlockRule* block(const QString &urlString) const;
    QList<const AdBlockRule*> pageRules() const { return m_pageRules; }

    QList<AdBlockRule> allRules() const;
    int addRule(const AdBlockRule &rule);
    void removeRule(int offset);
    void replaceRule(const AdBlockRule &rule, int offset);

private slots:
    void rulesDownloaded();

private:
    void populateCache();
    QString rulesFileName() const;
    void parseUrl(const QUrl &url);
    void loadRules();

    QString m_title;
    bool m_enabled;

    QNetworkReply* m_downloading;
    QList<AdBlockRule> m_rules;

    // sorted list
    QList<const AdBlockRule*> m_networkExceptionRules;
    QList<const AdBlockRule*> m_networkBlockRules;
    QList<const AdBlockRule*> m_pageRules;
};

#endif // ADBLOCKSUBSCRIPTION_H

