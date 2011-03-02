#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "qupzilla.h"
#include "webview.h"
#include "webpage.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    connect(ui->authorsButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    showAbout();
}

void AboutDialog::buttonClicked()
{
    if (ui->authorsButton->text() == tr("Authors and Contributors"))
        showAuthors();
    else if (ui->authorsButton->text() == tr("< About QupZilla"))
        showAbout();
}

void AboutDialog::showAbout()
{
    ui->authorsButton->setText(tr("Authors and Contributors"));
    if (m_aboutHtml.isEmpty()) {
        m_aboutHtml.append("<div style='margin:10px;'>");
        m_aboutHtml.append(tr("<p><b>Application version %1</b><br/>").arg(QupZilla::VERSION));
        m_aboutHtml.append(tr("<b>WebKit version %1</b></p>").arg(QupZilla::WEBKITVERSION));
        m_aboutHtml.append(tr("<p>&copy; %1 %2<br/>All rights reserved.<br/>").arg(QupZilla::COPYRIGHT, QupZilla::AUTHOR));
        m_aboutHtml.append(tr("Build time: %1 </p>").arg(QupZilla::BUILDTIME));
        m_aboutHtml.append(QString("<p><a href=%1>%1</a></p>").arg(QupZilla::WWWADDRESS));
        m_aboutHtml.append("<p>"+MainApplication::getInstance()->getWindow()->weView()->getPage()->userAgentForUrl(QUrl())+"</p>");
        m_aboutHtml.append("</div>");
    }
    ui->textBrowser->setHtml(m_aboutHtml);
}

void AboutDialog::showAuthors()
{
    ui->authorsButton->setText(tr("< About QupZilla"));
    if (m_authorsHtml.isEmpty()) {
        m_authorsHtml.append("<div style='margin:10px;'>");
        m_authorsHtml.append(tr("<p><b>Main developers:</b><br/>%1 &lt;%2&gt;</p>").arg(QupZilla::AUTHOR, "<a href=mailto:nowrep@gmail.com>nowrep@gmail.com</a>"));
        m_authorsHtml.append(tr("<p><b>Other contributors:</b><br/>%1</p>").arg("Rajny :: Graphics <br/> Mikino :: Slovakia Translation"));
        m_authorsHtml.append(tr("<p><b>Thanks to:</b><br/>%1</p>").arg("Patrick :: First User"));
        m_authorsHtml.append("</div>");
    }
    ui->textBrowser->setHtml(m_authorsHtml);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
