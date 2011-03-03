/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  nowrep
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
#include "clearprivatedata.h"
#include "qupzilla.h"
#include "cookiejar.h"
#include "mainapplication.h"
#include "networkmanager.h"
#include "clickablelabel.h"

ClearPrivateData::ClearPrivateData(QupZilla* mainClass, QWidget *parent) :
    QDialog(parent)
    ,p_QupZilla(mainClass)
{
    setWindowTitle(tr("Clear Recent History"));
    setWindowIcon(QIcon(":/icons/qupzilla.png"));
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_label = new QLabel(this);
    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->addButton(QDialogButtonBox::Ok);
    m_buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    m_layout->addWidget(m_label);
    m_label->setText(tr("Choose what you want to delete:"));
    m_layout->addWidget(m_buttonBox);

    m_clearHistory = new QCheckBox(tr("Clear history"), this);
    m_clearCookies = new QCheckBox(tr("Clear cookies"), this);
    m_clearCache = new QCheckBox(tr("Clear cache"), this);
    m_clearIcons = new QCheckBox(tr("Clear icons"), this);
    m_clearFlashCookies = new ClickableLabel(this);
    m_clearFlashCookies->setText(tr("Clear cookies from Adobe Flash Player"));

    m_clearHistory->setChecked(true);
    m_clearCookies->setChecked(true);
    m_clearCache->setChecked(true);
    m_clearIcons->setChecked(true);
    m_clearFlashCookies->setStyleSheet("color: blue; text-decoration: underline;");
    m_clearFlashCookies->setCursor(Qt::PointingHandCursor);

    m_layout->addWidget(m_clearHistory);
    m_layout->addWidget(m_clearCookies);
    m_layout->addWidget(m_clearCache);
    m_layout->addWidget(m_clearIcons);
    m_layout->addWidget(m_clearFlashCookies);

    m_layout->addWidget(m_buttonBox);

    connect(m_clearFlashCookies, SIGNAL(clicked(QPoint)), this, SLOT(clearFlash()));
}

void ClearPrivateData::clearFlash()
{
    p_QupZilla->loadAddress(QUrl("http://www.macromedia.com/support/documentation/en/flashplayer/help/settings_manager07.html"));
}

void ClearPrivateData::dialogAccepted()
{
    if (m_clearHistory->isChecked()) {
        QSqlQuery query;
        query.exec("DELETE FROM history");
        query.exec("VACUUM");
    }
    if (m_clearCookies->isChecked()) {
        QList<QNetworkCookie> cookies;
        p_QupZilla->getMainApp()->cookieJar()->setAllCookies(cookies);
    }
    if (m_clearCache->isChecked()) {
        p_QupZilla->getMainApp()->webSettings()->clearMemoryCaches();
        p_QupZilla->getMainApp()->networkManager()->cache()->clear();
    }
    if (m_clearIcons->isChecked()) {
        p_QupZilla->getMainApp()->webSettings()->clearIconDatabase();
    }
    close();
}
