/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2016 David Rosca <nowrep@gmail.com>
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
#include "autofillwidget.h"
#include "ui_autofillwidget.h"
#include "autofill.h"
#include "qztools.h"
#include "webview.h"
#include "webpage.h"
#include "scripts.h"

#include <QPushButton>

AutoFillWidget::AutoFillWidget(WebView* view, QWidget* parent)
    : LocationBarPopup(parent)
    , ui(new Ui::AutoFillWidget)
    , m_view(view)
{
    ui->setupUi(this);
}

void AutoFillWidget::setFormData(const QVector<PasswordEntry> &data)
{
    m_data = data;

    for (int i = 0; i < data.count(); ++i) {
        const PasswordEntry d = data.at(i);
        if (d.username.isEmpty()) {
            continue;
        }

        QPushButton* button = new QPushButton(this);
        button->setIcon(QIcon::fromTheme("user"), QIcon(":icons/other/login.png"));
        button->setStyleSheet("text-align:left;font-weight:bold;");
        button->setText(d.username);
        button->setProperty("data-index", i);
        button->setFlat(true);

        ui->gridLayout->addWidget(button, i, 0);
        connect(button, SIGNAL(clicked()), this, SLOT(loginToPage()));
    }
}

void AutoFillWidget::loginToPage()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button || !m_view) {
        return;
    }

    bool ok;
    int index = button->property("data-index").toInt(&ok);

    if (ok && QzTools::containsIndex(m_data, index)) {
        const PasswordEntry entry = m_data.at(index);
        m_view->page()->runJavaScript(Scripts::completeFormData(entry.data), WebPage::SafeJsWorld);
    }

    close();
}

AutoFillWidget::~AutoFillWidget()
{
    delete ui;
}
