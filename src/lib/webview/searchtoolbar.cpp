/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  David Rosca <nowrep@gmail.com>
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
#include "tabbedwebview.h"
#include "lineedit.h"
#include "ui_searchtoolbar.h"
#include "iconprovider.h"

#include <QKeyEvent>
#include <QShortcut>

SearchToolBar::SearchToolBar(WebView* view, QWidget* parent)
    : AnimatedWidget(AnimatedWidget::Up, 300, parent)
    , ui(new Ui::SearchToolbar)
    , m_view(view)
    , m_findFlags(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(widget());

    ui->closeButton->setIcon(qIconProvider->standardIcon(QStyle::SP_DialogCloseButton));
    ui->next->setIcon(qIconProvider->standardIcon(QStyle::SP_ArrowForward));
    ui->previous->setIcon(qIconProvider->standardIcon(QStyle::SP_ArrowBack));

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

    parent->installEventFilter(this);
}

void SearchToolBar::setWebView(WebView* view)
{
    m_view = view;
}

void SearchToolBar::showMinimalInPopupWindow()
{
    // Show only essentials widget + set minimum width
    ui->highligh->hide();
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

void SearchToolBar::hide()
{
    AnimatedWidget::hide();

    searchText(QString());
    m_view->setFocus();
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
    if (ui->highligh->isChecked()) {
        m_view->findText(ui->lineEdit->text(), m_findFlags | QWebPage::HighlightAllOccurrences);
    }
    else {
        m_view->findText(QString(), QWebPage::HighlightAllOccurrences);
    }
}

void SearchToolBar::caseSensitivityChanged()
{
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolBar::searchText(const QString &text)
{
    // Clear highlighting on page
    m_view->findText(QString(), QWebPage::HighlightAllOccurrences);

    bool found = m_view->findText(text, m_findFlags);

    if (text.isEmpty()) {
        found = true;
    }

    if (ui->highligh->isChecked()) {
        m_findFlags = QWebPage::HighlightAllOccurrences;
        updateFindFlags();
        m_view->findText(text, m_findFlags);
    }
    else {
        m_view->findText(QString(), QWebPage::HighlightAllOccurrences);
    }

    if (!found) {
        ui->results->setText(tr("No results found."));
    }
    else {
        ui->results->clear();
    }


    ui->lineEdit->setProperty("notfound", QVariant(!found));

    ui->lineEdit->style()->unpolish(ui->lineEdit);
    ui->lineEdit->style()->polish(ui->lineEdit);
}

bool SearchToolBar::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        hide();
    }

    return false;
}

SearchToolBar::~SearchToolBar()
{
    delete ui;
}
