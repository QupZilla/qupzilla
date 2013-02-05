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
#include "sourceviewer.h"
#include "tabbedwebview.h"
#include "htmlhighlighter.h"
#include "sourceviewersearch.h"
#include "qztools.h"
#include "iconprovider.h"
#include "enhancedmenu.h"
#include "plaineditwithlines.h"

#include <QBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QWebFrame>
#include <QTimer>

SourceViewer::SourceViewer(QWebFrame* frame, const QString &selectedHtml)
    : QWidget(0)
    , m_frame(frame)
    , m_selectedHtml(selectedHtml)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Source of ") + frame->url().toString());
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_sourceEdit = new PlainEditWithLines(this);
    m_sourceEdit->setObjectName("sourceviewer-textedit");
    m_sourceEdit->setReadOnly(true);
    m_sourceEdit->setUndoRedoEnabled(false);

    m_statusBar = new QStatusBar(this);
    m_statusBar->showMessage(frame->url().toString());

    QMenuBar* menuBar = new QMenuBar(this);
    m_layout->addWidget(m_sourceEdit);
    m_layout->addWidget(m_statusBar);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setMenuBar(menuBar);

    QFont font;
    font.setFamily("Tahoma");
    font.setFixedPitch(true);
    font.setPointSize(10);

    m_sourceEdit->setFont(font);
    new HtmlHighlighter(m_sourceEdit->document());

    resize(650, 600);
    QzTools::centerWidgetToParent(this, frame->page()->view());

    QMenu* menuFile = new QMenu(tr("File"));
    menuFile->addAction(tr("Load in page"), this, SLOT(loadInPage()));
    menuFile->addAction(QIcon::fromTheme("document-save"), tr("Save as..."), this, SLOT(save()))->setShortcut(QKeySequence("Ctrl+S"));
    menuFile->addSeparator();
    menuFile->addAction(QIcon::fromTheme("window-close"), tr("Close"), this, SLOT(close()))->setShortcut(QKeySequence("Ctrl+W"));
    menuBar->addMenu(menuFile);

    QMenu* menuEdit = new QMenu(tr("Edit"));
    m_actionUndo = menuEdit->addAction(QIcon::fromTheme("edit-undo"), tr("Undo"), m_sourceEdit, SLOT(undo()));
    m_actionRedo = menuEdit->addAction(QIcon::fromTheme("edit-redo"), tr("Redo"), m_sourceEdit, SLOT(redo()));
    menuEdit->addSeparator();
    m_actionCut = menuEdit->addAction(QIcon::fromTheme("edit-cut"), tr("Cut"), m_sourceEdit, SLOT(cut()));
    m_actionCopy = menuEdit->addAction(QIcon::fromTheme("edit-copy"), tr("Copy"), m_sourceEdit, SLOT(copy()));
    m_actionPaste = menuEdit->addAction(QIcon::fromTheme("edit-paste"), tr("Paste"), m_sourceEdit, SLOT(paste()));
    menuEdit->addSeparator();
    menuEdit->addAction(QIcon::fromTheme("edit-select-all"), tr("Select All"), m_sourceEdit, SLOT(selectAll()))->setShortcut(QKeySequence("Ctrl+A"));
    menuEdit->addAction(QIcon::fromTheme("edit-find"), tr("Find"), this, SLOT(findText()))->setShortcut(QKeySequence("Ctrl+F"));
    menuEdit->addSeparator();
    menuEdit->addAction(QIcon::fromTheme("go-jump"), tr("Go to Line..."), this, SLOT(goToLine()))->setShortcut(QKeySequence("Ctrl+L"));
    menuBar->addMenu(menuEdit);

    m_actionUndo->setShortcut(QKeySequence("Ctrl+Z"));
    m_actionRedo->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    m_actionCut->setShortcut(QKeySequence("Ctrl+X"));
    m_actionCopy->setShortcut(QKeySequence("Ctrl+C"));
    m_actionPaste->setShortcut(QKeySequence("Ctrl+V"));

    QMenu* menuView = new QMenu(tr("View"));
    menuView->addAction(qIconProvider->standardIcon(QStyle::SP_BrowserReload), tr("Reload"), this, SLOT(reload()))->setShortcut(QKeySequence("F5"));
    menuView->addSeparator();
    menuView->addAction(tr("Editable"), this, SLOT(setTextEditable()))->setCheckable(true);
    menuView->addAction(tr("Word Wrap"), this, SLOT(setTextWordWrap()))->setCheckable(true);
    menuView->actions().at(3)->setChecked(true);
    menuBar->addMenu(menuView);

    connect(m_sourceEdit, SIGNAL(copyAvailable(bool)), this, SLOT(copyAvailable(bool)));
    connect(m_sourceEdit, SIGNAL(redoAvailable(bool)), this, SLOT(redoAvailable(bool)));
    connect(m_sourceEdit, SIGNAL(undoAvailable(bool)), this, SLOT(undoAvailable(bool)));
    connect(menuEdit, SIGNAL(aboutToShow()), this, SLOT(pasteAvailable()));

    QTimer::singleShot(0, this, SLOT(loadSource()));
}

void SourceViewer::copyAvailable(bool yes)
{
    m_actionCopy->setEnabled(yes);
    m_actionCut->setEnabled(yes);
}

void SourceViewer::redoAvailable(bool available)
{
    m_actionRedo->setEnabled(available);
}

void SourceViewer::undoAvailable(bool available)
{
    m_actionUndo->setEnabled(available);
}

void SourceViewer::pasteAvailable()
{
    m_actionPaste->setEnabled(m_sourceEdit->canPaste());
}

void SourceViewer::loadInPage()
{
    if (m_frame) {
        m_frame.data()->setHtml(m_sourceEdit->toPlainText(), m_frame.data()->baseUrl());
        m_statusBar->showMessage(tr("Source loaded in page"));
    }
    else {
        m_statusBar->showMessage(tr("Cannot load in page. Page has been closed."));
    }
}

void SourceViewer::loadSource()
{
    m_actionUndo->setEnabled(false);
    m_actionRedo->setEnabled(false);
    m_actionCut->setEnabled(false);
    m_actionCopy->setEnabled(false);
    m_actionPaste->setEnabled(false);

    QString html = m_frame.data()->toHtml();
    // Remove AdBlock element hiding rules
    html.remove(QRegExp("<style type=\"text/css\">\n/\\* AdBlock for QupZilla \\*/\n.*\\{display: none !important;\\}\n</style>"));
    m_sourceEdit->setPlainText(html);

    // Highlight selectedHtml
    if (!m_selectedHtml.isEmpty()) {
        m_sourceEdit->find(m_selectedHtml, QTextDocument::FindWholeWords);
    }

    m_sourceEdit->setShowingCursor(true);
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
    if (m_frame) {
        m_sourceEdit->clear();
        loadSource();

        m_statusBar->showMessage(tr("Source reloaded"));
    }
    else {
        m_statusBar->showMessage(tr("Cannot reload source. Page has been closed."));
    }
}

void SourceViewer::setTextEditable()
{
    m_sourceEdit->setReadOnly(!m_sourceEdit->isReadOnly());
    m_sourceEdit->setUndoRedoEnabled(true);

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

    m_sourceEdit->goToLine(line);
}
