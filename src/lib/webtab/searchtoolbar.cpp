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
#include "searchtoolbar.h"
#include "webview.h"
#include "webpage.h"
#include "lineedit.h"
#include "ui_searchtoolbar.h"
#include "iconprovider.h"

#include <QKeyEvent>
#include <QShortcut>

SearchToolBar::SearchToolBar(WebView* view, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SearchToolbar)
    , m_view(view)
    , m_findFlags(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->closeButton->setIcon(IconProvider::instance()->standardIcon(QStyle::SP_DialogCloseButton));
    ui->next->setIcon(IconProvider::instance()->standardIcon(QStyle::SP_ArrowDown));
    ui->next->setShortcut(QKeySequence("Ctrl+G"));
    ui->previous->setIcon(IconProvider::instance()->standardIcon(QStyle::SP_ArrowUp));
    ui->previous->setShortcut(QKeySequence("Ctrl+Shift+G"));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(findNext()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
    connect(ui->next, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(ui->previous, SIGNAL(clicked()), this, SLOT(findPrevious()));
    connect(ui->caseSensitive, SIGNAL(clicked()), this, SLOT(caseSensitivityChanged()));

    QShortcut* findNextAction = new QShortcut(QKeySequence("F3"), this);
    connect(findNextAction, SIGNAL(activated()), this, SLOT(findNext()));

    QShortcut* findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);
    connect(findPreviousAction, SIGNAL(activated()), this, SLOT(findPrevious()));

    parent->installEventFilter(this);
}

void SearchToolBar::showMinimalInPopupWindow()
{
    // Show only essentials widget + set minimum width
    ui->caseSensitive->hide();
    ui->results->hide();
    ui->horizontalLayout->setSpacing(2);
    ui->horizontalLayout->setContentsMargins(2, 6, 2, 6);
    setMinimumWidth(260);
}

void SearchToolBar::focusSearchLine()
{
    ui->lineEdit->setFocus();
}

void SearchToolBar::close()
{
    hide();
    searchText(QString());
    m_view->setFocus();
    deleteLater();
}

void SearchToolBar::findNext()
{
    m_findFlags = 0;
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::findPrevious()
{
    m_findFlags = QWebEnginePage::FindBackward;
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::updateFindFlags()
{
    if (ui->caseSensitive->isChecked()) {
        m_findFlags = m_findFlags | QWebEnginePage::FindCaseSensitively;
    }
    else {
        m_findFlags = m_findFlags & ~QWebEnginePage::FindCaseSensitively;
    }
}

void SearchToolBar::caseSensitivityChanged()
{
    updateFindFlags();

    searchText(QString());
    searchText(ui->lineEdit->text());
}

void SearchToolBar::setText(const QString &text)
{
    ui->lineEdit->setText(text);
}

void SearchToolBar::searchText(const QString &text)
{
    QPointer<SearchToolBar> guard = this;
    m_view->findText(text, m_findFlags, [=](bool found) {
        if (!guard) {
            return;
        }
        if (ui->lineEdit->text().isEmpty())
            found = true;

        if (!found)
            ui->results->setText(tr("No results found."));
        else
            ui->results->clear();

        ui->lineEdit->setProperty("notfound", QVariant(!found));
        ui->lineEdit->style()->unpolish(ui->lineEdit);
        ui->lineEdit->style()->polish(ui->lineEdit);

        // Clear selection
        m_view->page()->runJavaScript(QSL("window.getSelection().empty();"), WebPage::SafeJsWorld);
    });
}

bool SearchToolBar::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        close();
    }

    return false;
}

SearchToolBar::~SearchToolBar()
{
    delete ui;
}
