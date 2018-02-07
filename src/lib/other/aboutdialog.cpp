/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#include "browserwindow.h"
#include "mainapplication.h"
#include "webpage.h"
#include "useragentmanager.h"

#include <QWebEnginePage>
#include <QWebEngineProfile>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
    , m_showingAuthors(false)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->label->setPixmap(QIcon(QSL(":icons/other/about.png")).pixmap(300, 130));

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->authorsButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    showAbout();
}

void AboutDialog::buttonClicked()
{
    if (m_showingAuthors)
        showAbout();
    else
        showAuthors();
}

void AboutDialog::showAbout()
{
    m_showingAuthors = false;
    ui->authorsButton->setText(tr("Authors and Contributors"));
    if (m_aboutHtml.isEmpty()) {
        m_aboutHtml += "<center><div style='margin:20px;'>";
        m_aboutHtml += tr("<p><b>Application version %1</b><br/>").arg(
#ifdef GIT_REVISION
                           QString("%1 (%2)").arg(Qz::VERSION, GIT_REVISION)
#else
                           Qz::VERSION
#endif
                       );
        m_aboutHtml += tr("<b>QtWebEngine version %1</b></p>").arg(qVersion());
        m_aboutHtml += QString("<p>&copy; %1 %2<br/>").arg(Qz::COPYRIGHT, Qz::AUTHOR);
        m_aboutHtml += QString("<a href=%1>%1</a></p>").arg(Qz::WWWADDRESS);
        m_aboutHtml += "<p>" + mApp->userAgentManager()->defaultUserAgent() + "</p>";
        m_aboutHtml += "</div></center>";
    }
    ui->textBrowser->setHtml(m_aboutHtml);
}

void AboutDialog::showAuthors()
{
    m_showingAuthors = true;
    ui->authorsButton->setText(tr("< About QupZilla"));
    if (m_authorsHtml.isEmpty()) {
        m_authorsHtml += "<center><div style='margin:10px;'>";
        m_authorsHtml += tr("<p><b>Main developer:</b><br/>%1 &lt;%2&gt;</p>").arg(Qz::AUTHOR, "<a href=mailto:nowrep@gmail.com>nowrep@gmail.com</a>");
        m_authorsHtml += tr("<p><b>Contributors:</b><br/>%1</p>").arg(
                             QString::fromUtf8("Mladen Pejaković<br/>"
                                               "Seyyed Razi Alavizadeh<br/>"
                                               "Adrien Vigneron<br/>"
                                               "Elio Qoshi<br/>"
                                               "Alexander Samilov<br/>"
                                               "Franz Fellner<br/>"
                                               "Bryan M Dunsmore<br/>"
                                               "Mariusz Fik<br/>"
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
                                               "Piccoro McKay Lenz<br/>"
                                               "Stanislav Kuznietsov<br/>"
                                               "Seyyed Razi Alavizadeh<br/>"
                                               "Guillem Prats<br/>"
                                               "Clara Villalba<br/>"
                                               "Yu Hai<br/>"
                                               "Muhammad Fawwaz Orabi<br/>"
                                               "Lasso Kante<br/>"
                                               "Kizito Birabwa<br/>"
                                               "Juan Carlos Sánchez<br/>"
                                               "Xabier Aramendi<br/>"
                                               "Ferhat AYDIN")
                         );
        m_authorsHtml += "</div></center>";
    }

    ui->textBrowser->setHtml(m_authorsHtml);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
