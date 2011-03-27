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

#include "adblockpage.h"

#include "adblockmanager.h"
#include "adblocksubscription.h"
#include "adblockrule.h"

#if QT_VERSION >= 0x040600
#include <qwebelement.h>
#endif
#include <qwebpage.h>
#include <qwebframe.h>

#include <qdebug.h>

// #define ADBLOCKPAGE_DEBUG

AdBlockPage::AdBlockPage(QObject *parent)
    : QObject(parent)
{
}

void AdBlockPage::checkRule(const AdBlockRule *rule, QWebPage *page, const QString &host)
{
    if (!rule->isEnabled())
        return;

    QString filter = rule->filter();
    int offset = filter.indexOf(QLatin1String("##"));
    if (offset == -1)
        return;

    QString selectorQuery;
    if (offset > 0) {
        QString domainRules = filter.mid(0, offset);
        selectorQuery = filter.mid(offset + 2);
        QStringList domains = domainRules.split(QLatin1Char(','));

        bool match = false;
        foreach (const QString &domain, domains) {
            bool reverse = (domain[0] == QLatin1Char('~'));
            if (reverse) {
                QString xdomain = domain.mid(1);
                if (host.endsWith(xdomain))
                    return;
                match = true;
            }
            if (host.endsWith(domain))
                match = true;
        }
        if (!match)
            return;
    }

    if (offset == 0)
        selectorQuery = filter.mid(2);

    Q_UNUSED(page);
#if QT_VERSION >= 0x040600
    QWebElement document = page->mainFrame()->documentElement();
    QWebElementCollection elements = document.findAll(selectorQuery);
#if defined(ADBLOCKPAGE_DEBUG)
    if (elements.count() != 0)
        qDebug() << "AdBlockPage::" << __FUNCTION__ << "blocking" << elements.count() << "items" << selectorQuery << elements.count() << "rule:" << rule->filter();
#endif
    foreach (QWebElement element, elements) {
        element.setStyleProperty(QLatin1String("visibility"), QLatin1String("hidden"));
        element.removeFromDocument();
    }

#endif
}

void AdBlockPage::applyRulesToPage(QWebPage *page)
{
    if (!page || !page->mainFrame())
        return;
    AdBlockManager *manager = AdBlockManager::instance();
    if (!manager->isEnabled())
        return;
#if QT_VERSION >= 0x040600
    QString host = page->mainFrame()->url().host();
    AdBlockSubscription* subscription = manager->subscription();

    QList<const AdBlockRule*> rules = subscription->pageRules();
        foreach (const AdBlockRule *rule, rules) {
            checkRule(rule, page, host);
        }
#endif
}

