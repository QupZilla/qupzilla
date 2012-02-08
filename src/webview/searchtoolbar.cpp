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
#include "searchtoolbar.h"
#include "qupzilla.h"
#include "tabbedwebview.h"
#include "lineedit.h"
#include "ui_searchtoolbar.h"
#include "iconprovider.h"

SearchToolBar::SearchToolBar(QupZilla* mainClass, QWidget* parent)
    : AnimatedWidget(AnimatedWidget::Up, 300, parent)
    , ui(new Ui::SearchToolbar)
    , p_QupZilla(mainClass)
    , m_findFlags(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(widget());

    ui->closeButton->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));
    ui->next->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));
    ui->previous->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(findNext()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
    connect(ui->next, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(ui->previous, SIGNAL(clicked()), this, SLOT(findPrevious()));
    connect(ui->highligh, SIGNAL(clicked()), this, SLOT(highlightChanged()));
    connect(ui->caseSensitive, SIGNAL(clicked()), this, SLOT(caseSensitivityChanged()));
    startAnimation();

    QShortcut* findNextAction = new QShortcut(QKeySequence("F3"), this);
    connect(findNextAction, SIGNAL(activated()), this, SLOT(findNext()));

    QShortcut* findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);
    connect(findPreviousAction, SIGNAL(activated()), this, SLOT(findPrevious()));

    qApp->installEventFilter(this);
}

QLineEdit* SearchToolBar::searchLine()
{
    return ui->lineEdit;
}

void SearchToolBar::hide()
{
    searchText("");
    p_QupZilla->weView()->setFocus();

    AnimatedWidget::hide();
}

void SearchToolBar::findNext()
{
    m_findFlags = QWebPage::FindWrapsAroundDocument;
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::findPrevious()
{
    m_findFlags = QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument;
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::updateFindFlags()
{
    if (ui->caseSensitive->isChecked()) {
        m_findFlags = m_findFlags | QWebPage::FindCaseSensitively;
    }
    else {
        m_findFlags = m_findFlags & ~QWebPage::FindCaseSensitively;
    }
}

void SearchToolBar::highlightChanged()
{
    WebView* view = p_QupZilla->weView();

    if (ui->highligh->isChecked()) {
        view->findText(ui->lineEdit->text(), m_findFlags | QWebPage::HighlightAllOccurrences);
    }
    else {
        view->findText("", QWebPage::HighlightAllOccurrences);
    }
}

void SearchToolBar::caseSensitivityChanged()
{
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::searchText(const QString &text)
{
    WebView* view = p_QupZilla->weView();
    bool found = view->findText(text, m_findFlags);
    if (text.isEmpty()) {
        found = true;
    }

    if (ui->highligh->isChecked()) {
        m_findFlags = QWebPage::HighlightAllOccurrences;
        updateFindFlags();
        view->findText(text, m_findFlags);
    }
    else {
        view->findText("", QWebPage::HighlightAllOccurrences);
    }

    if (!found) {
        ui->results->setText(tr("No results found."));
    }
    else {
        ui->results->clear();
    }


    ui->lineEdit->setProperty("notfound", !found);

    ui->lineEdit->style()->unpolish(ui->lineEdit);
    ui->lineEdit->style()->polish(ui->lineEdit);
}

bool SearchToolBar::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        hide();
        return false;
    }

    return AnimatedWidget::eventFilter(obj, event);
}

SearchToolBar::~SearchToolBar()
{
    delete ui;
}
