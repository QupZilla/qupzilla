/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "qupzilla.h"
#include "mainapplication.h"
#include "tabbedwebview.h"
#include "webpage.h"
#include "qtwin.h"
#include <QDebug>
AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
#ifdef Q_WS_WIN
    if (QtWin::isCompositionEnabled()) {
        QtWin::extendFrameIntoClientArea(this);
        ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    }
#endif

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->authorsButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    showAbout();
}

void AboutDialog::buttonClicked()
{
    if (ui->authorsButton->text() == tr("Authors and Contributors")) {
        showAuthors();
    }
    else if (ui->authorsButton->text() == tr("< About QupZilla")) {
        showAbout();
    }
}

void AboutDialog::showAbout()
{
    ui->authorsButton->setText(tr("Authors and Contributors"));
    if (m_aboutHtml.isEmpty()) {
        m_aboutHtml += "<center><div style='margin:10px;'>";
        m_aboutHtml += tr("<p><b>Application version %1</b><br/>").arg(QupZilla::VERSION
#ifdef GIT_REVISION
                       + " (" + GIT_REVISION + ")"
#endif
                                                                      );
        m_aboutHtml += tr("<b>WebKit version %1</b></p>").arg(QupZilla::WEBKITVERSION);
        m_aboutHtml += QString("<p>&copy; %1 %2<br/>").arg(QupZilla::COPYRIGHT, QupZilla::AUTHOR);
        m_aboutHtml += tr("<small>Build time: %1 </small></p>").arg(QupZilla::BUILDTIME);
        m_aboutHtml += QString("<p><a href=%1>%1</a></p>").arg(QupZilla::WWWADDRESS);
        m_aboutHtml += "<p>" + mApp->getWindow()->weView()->webPage()->userAgentForUrl(QUrl()) + "</p>";
        m_aboutHtml += "</div></center>";
    }
    ui->textBrowser->setHtml(m_aboutHtml);
}

void AboutDialog::showAuthors()
{
    ui->authorsButton->setText(tr("< About QupZilla"));
    if (m_authorsHtml.isEmpty()) {
        m_authorsHtml += "<center><div style='margin:10px;'>";
        m_authorsHtml += tr("<p><b>Main developer:</b><br/>%1 &lt;%2&gt;</p>").arg(QupZilla::AUTHOR, "<a href=mailto:nowrep@gmail.com>nowrep@gmail.com</a>");
        m_authorsHtml += tr("<p><b>Contributors:</b><br/>%1</p>").arg(
                             QString::fromUtf8("Mladen Pejaković<br/>"
                                               "Bryan M Dunsmore<br/>"
                                               "Mariusz Fik<br/>"
                                               "Jan Rajnoha<br/>"
                                               "Daniele Cocca")
                         );

        m_authorsHtml += tr("<p><b>Translators:</b><br/>%1</p>").arg(
                             QString::fromUtf8("Heimen Stoffels<br/>"
                                               "Peter Vacula<br/>"
                                               "Jonathan Hooverman<br/>"
                                               "Federico Fabiani<br/>"
                                               "Francesco Marinucci<br/>"
                                               "Jorge Sevilla<br/>"
                                               "Ștefan Comănescu<br/>"
                                               "Michał Szymanowski<br/>"
                                               "Mariusz Fik<br/>"
                                               "Jérôme Giry<br/>"
                                               "Nicolas Ourceau<br/>"
                                               "Vasilis Tsivikis<br/>"
                                               "Rustam Salakhutdinov<br/>"
                                               "Oleg Brezhnev<br/>"
                                               "Sérgio Marques<br/>"
                                               "Alexandre Carvalho<br/>"
                                               "Mladen Pejaković<br/>"
                                               "Unink-Lio<br/>"
                                               "Wu Cheng-Hong<br/>"
                                               "Widya Walesa<br/>"
                                               "Beqa Arabuli<br/>"
                                               "Daiki Noda<br/>"
                                               "Gábor Oberle<br/>"
                                               "Piccoro McKay Lenz")
                         );
        m_authorsHtml += "</div></center>";
    }

    ui->textBrowser->setHtml(m_authorsHtml);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
