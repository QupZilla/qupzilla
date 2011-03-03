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
#include "sourceviewer.h"
#include "qupzilla.h"
#include "webview.h"
SourceViewer::SourceViewer(QupZilla* mainClass, QWidget *parent) :
    QWidget(parent)
    ,p_QupZilla(mainClass)
{
    setWindowTitle(tr("Source of ")+p_QupZilla->weView()->url().toString());
    setWindowIcon(QIcon(":/icons/qupzilla.png"));
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_sourceEdit = new QTextEdit(this);

    m_layout->addWidget(m_sourceEdit);
    m_layout->setContentsMargins(1, 2, 1, 2);

    m_sourceEdit->insertPlainText(p_QupZilla->weView()->page()->mainFrame()->toHtml());

    this->resize(650, 600);
    m_sourceEdit->setReadOnly(true);
    m_sourceEdit->moveCursor(QTextCursor::Start);
    //CENTER on scren
    const QRect screen = QApplication::desktop()->screenGeometry();
    const QRect &size = QWidget::geometry();
    QWidget::move( (screen.width()-size.width())/2, (screen.height()-size.height())/2 );
}
