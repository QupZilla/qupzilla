/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#include "sourceviewersearch.h"
#include "ui_sourceviewersearch.h"
#include "sourceviewer.h"
#include "iconprovider.h"
#include "plaineditwithlines.h"

#include <QShortcut>
#include <QKeyEvent>

#if QTWEBENGINE_DISABLED

SourceViewerSearch::SourceViewerSearch(SourceViewer* parent)
    : AnimatedWidget(AnimatedWidget::Up)
    , m_sourceViewer(parent)
    , ui(new Ui::SourceViewerSearch)
    , m_findFlags(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(widget());
    ui->closeButton->setIcon(IconProvider::standardIcon(QStyle::SP_DialogCloseButton));

    ui->next->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowForward));

    ui->previous->setIcon(IconProvider::standardIcon(QStyle::SP_ArrowBack));
    ui->lineEdit->setFocus();
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(next()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->next, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->previous, SIGNAL(clicked()), this, SLOT(previous()));
    connect(ui->wholeWords, SIGNAL(toggled(bool)), SLOT(searchWholeWords()));
    connect(this, SIGNAL(performSearch()), SLOT(find()));

    QShortcut* findNextAction = new QShortcut(QKeySequence("F3"), this);
    connect(findNextAction, SIGNAL(activated()), this, SLOT(next()));

    QShortcut* findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);
    connect(findPreviousAction, SIGNAL(activated()), this, SLOT(previous()));

    startAnimation();
    parent->installEventFilter(this);
}

void SourceViewerSearch::activateLineEdit()
{
    ui->lineEdit->setFocus();
}

void SourceViewerSearch::next()
{
    m_findFlags &= (~QTextDocument::FindBackward);
    emit performSearch();
}

void SourceViewerSearch::previous()
{
    m_findFlags |= QTextDocument::FindBackward;
    emit performSearch();
}

void SourceViewerSearch::searchWholeWords()
{
    if (ui->wholeWords->isChecked()) {
        m_findFlags |= QTextDocument::FindWholeWords;
    }
    else {
        m_findFlags &= (~QTextDocument::FindWholeWords);
    }
}

void SourceViewerSearch::find()
{
    bool found = find(m_findFlags);

    if (!found) {
        m_sourceViewer->sourceEdit()->moveCursor(QTextCursor::Start);
    }

    ui->lineEdit->setProperty("notfound", QVariant(!found));

    ui->lineEdit->style()->unpolish(ui->lineEdit);
    ui->lineEdit->style()->polish(ui->lineEdit);
}

bool SourceViewerSearch::find(QTextDocument::FindFlags flags)
{
    QString string = ui->lineEdit->text();
    if (string.isEmpty()) {
        return true;
    }
    if (string != m_lastSearchedString) {
        QTextCursor cursor = m_sourceViewer->sourceEdit()->textCursor();
        cursor.setPosition(cursor.selectionStart());
        cursor.clearSelection();
        m_sourceViewer->sourceEdit()->setTextCursor(cursor);
        m_lastSearchedString = string;
    }

    if (!m_sourceViewer->sourceEdit()->find(string, flags)) {
        QTextCursor cursor = m_sourceViewer->sourceEdit()->textCursor();
        m_sourceViewer->sourceEdit()->moveCursor((flags == QTextDocument::FindBackward) ? QTextCursor::End : QTextCursor::Start);
        if (!m_sourceViewer->sourceEdit()->find(string, flags)) {
            cursor.clearSelection();
            m_sourceViewer->sourceEdit()->setTextCursor(cursor);
            return false;
        }
    }
    return true;
}

bool SourceViewerSearch::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        hide();
        return false;
    }

    return AnimatedWidget::eventFilter(obj, event);
}

#endif
