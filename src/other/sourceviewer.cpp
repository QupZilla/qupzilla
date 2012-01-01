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
#include "sourceviewer.h"
#include "webview.h"
#include "htmlhighlighter.h"
#include "sourceviewersearch.h"
#include "globalfunctions.h"
#include "iconprovider.h"
#include "enhancedmenu.h"

SourceViewer::SourceViewer(QWebPage* page, const QString &selectedHtml)
    : QWidget(0)
    , m_page(page)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Source of ") + page->mainFrame()->url().toString());
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_sourceEdit = new QTextEdit(this);
    m_sourceEdit->setObjectName("sourceviewer-textedit");

    m_statusBar = new QStatusBar(this);
    m_statusBar->showMessage(page->mainFrame()->url().toString());
    QMenuBar* menuBar = new QMenuBar(this);
    m_layout->addWidget(m_sourceEdit);
    m_layout->addWidget(m_statusBar);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setMenuBar(menuBar);

    this->resize(650, 600);
    m_sourceEdit->setReadOnly(true);
    m_sourceEdit->moveCursor(QTextCursor::Start);

    QFont font;
    font.setFamily("Tahoma");
    font.setFixedPitch(true);
    font.setPointSize(10);

    m_sourceEdit->setFont(font);
    new HtmlHighlighter(m_sourceEdit->document());

    QMenu* menuFile = new QMenu(tr("File"));
    menuFile->addAction(QIcon::fromTheme("document-save"), tr("Save as..."), this, SLOT(save()))->setShortcut(QKeySequence("Ctrl+S"));
    menuFile->addSeparator();
    menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+W"));
    menuBar->addMenu(menuFile);

    QMenu* menuEdit = new QMenu(tr("Edit"));
    menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("Undo"), m_sourceEdit, SLOT(undo()))->setShortcut(QKeySequence("Ctrl+Z"));
    menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("Redo"), m_sourceEdit, SLOT(redo()))->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    menuEdit->addSeparator();
    menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("Cut"), m_sourceEdit, SLOT(cut()))->setShortcut(QKeySequence("Ctrl+X"));
    menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), m_sourceEdit, SLOT(copy()))->setShortcut(QKeySequence("Ctrl+C"));
    menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"), m_sourceEdit, SLOT(paste()))->setShortcut(QKeySequence("Ctrl+V"));
    menuEdit->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"))->setShortcut(QKeySequence("Del"));
    menuEdit->addSeparator();
    menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All"), m_sourceEdit, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("Find"), this, SLOT(findText()))->setShortcut(QKeySequence("Ctrl+F"));
    menuEdit->addSeparator();
    menuEdit->addAction(QIcon::fromTheme("go-jump"), tr("Go to Line..."), this, SLOT(goToLine()))->setShortcut(QKeySequence("Ctrl+L"));
    menuBar->addMenu(menuEdit);

    QMenu* menuView = new QMenu(tr("View"));
    menuView->addAction(IconProvider::standardIcon(QStyle::SP_BrowserReload), tr("Reload"), this, SLOT(reload()))->setShortcut(QKeySequence("F5"));
    menuView->addSeparator();
    menuView->addAction(tr("Editable"), this, SLOT(setTextEditable()))->setCheckable(true);
    menuView->addAction(tr("Word Wrap"), this, SLOT(setTextWordWrap()))->setCheckable(true);
    menuView->actions().at(3)->setChecked(true);
    menuBar->addMenu(menuView);

    qz_centerWidgetToParent(this, page->view());

    m_sourceEdit->setUndoRedoEnabled(false);
    m_sourceEdit->insertPlainText(m_page->mainFrame()->toHtml());
    m_sourceEdit->setUndoRedoEnabled(true);

    //Highlight selectedHtml
    if (!selectedHtml.isEmpty()) {
        m_sourceEdit->find(selectedHtml, QTextDocument::FindWholeWords);
    }
    else {
        QTextCursor cursor = m_sourceEdit->textCursor();
        cursor.setPosition(0);
        m_sourceEdit->setTextCursor(cursor);
    }
}

void SourceViewer::save()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save file..."), QDir::homePath() + "/source_code.html");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("Error!"), tr("Cannot write to file!"));
        m_statusBar->showMessage(tr("Error writing to file"));
        return;
    }
    file.write(m_sourceEdit->toPlainText().toUtf8());
    file.close();

    m_statusBar->showMessage(tr("Source successfully saved"));
}

void SourceViewer::findText()
{
    if (m_layout->count() > 2) {
        SourceViewerSearch* search = qobject_cast<SourceViewerSearch*>(m_layout->itemAt(1)->widget());
        search->activateLineEdit();
        return;
    }

    SourceViewerSearch* s = new SourceViewerSearch(this);
    m_layout->insertWidget(1, s);
    s->activateLineEdit();
}

void SourceViewer::reload()
{
    m_sourceEdit->clear();
    m_sourceEdit->insertPlainText(m_page->mainFrame()->toHtml());

    m_statusBar->showMessage(tr("Source reloaded"));
}

void SourceViewer::setTextEditable()
{
    m_sourceEdit->setReadOnly(!m_sourceEdit->isReadOnly());

    m_statusBar->showMessage(tr("Editable changed"));
}

void SourceViewer::setTextWordWrap()
{
    m_sourceEdit->setWordWrapMode((m_sourceEdit->wordWrapMode() == QTextOption::NoWrap) ? QTextOption::WordWrap : QTextOption::NoWrap);

    m_statusBar->showMessage(tr("Word Wrap changed"));
}

void SourceViewer::goToLine()
{
    int line = QInputDialog::getInt(this, tr("Go to Line..."), tr("Enter line number"), 0, 1, 5000);
    if (line == 0) {
        return;
    }

    m_sourceEdit->setUpdatesEnabled(false);
    m_sourceEdit->moveCursor(QTextCursor::Start);
    for (int i = 0; i < line; i++) {
        m_sourceEdit->moveCursor(QTextCursor::Down);
    }
    m_sourceEdit->setUpdatesEnabled(true);
}
