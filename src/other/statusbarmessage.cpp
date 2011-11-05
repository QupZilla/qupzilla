/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include "statusbarmessage.h"
#include "qupzilla.h"
#include "squeezelabelv1.h"

TipLabel::TipLabel(QupZilla* parent)
    : SqueezeLabelV1(parent)
    , p_QupZilla(parent)
    , m_connected(false)
{
    setWindowFlags(Qt::ToolTip);
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    setFrameStyle(QFrame::NoFrame);
    setMargin(3);

    qApp->installEventFilter(this);
}

void TipLabel::show()
{
    if (!m_connected) {
        connect(p_QupZilla->tabWidget(), SIGNAL(currentChanged(int)), this, SLOT(hide()));
        m_connected = true;
    }

    SqueezeLabelV1::show();
}

void TipLabel::paintEvent(QPaintEvent* ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    SqueezeLabelV1::paintEvent(ev);
}

bool TipLabel::eventFilter(QObject* o, QEvent* e)
{
    Q_UNUSED(o);

    switch (e->type()) {
    case QEvent::Leave:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hide();
        break;

    default:
        break;
    }
    return false;
}

StatusBarMessage::StatusBarMessage(QupZilla* mainClass)
    : QObject(mainClass)
    , p_QupZilla(mainClass)
    , m_statusBarText(new TipLabel(mainClass))
{
    m_statusBarText->setParent(p_QupZilla);
}

void StatusBarMessage::showMessage(const QString &message)
{
    if (p_QupZilla->statusBar()->isVisible())
        p_QupZilla->statusBar()->showMessage(message);
    else {
        WebView* view = p_QupZilla->weView();
        QWebFrame* mainFrame = view->page()->mainFrame();

        int horizontalScrollSize = 0;
        int verticalScrollSize = 0;
        const int scrollbarSize = p_QupZilla->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

        if (mainFrame->scrollBarMaximum(Qt::Horizontal))
            horizontalScrollSize = scrollbarSize;
        if (mainFrame->scrollBarMaximum(Qt::Vertical))
            verticalScrollSize = scrollbarSize;

        m_statusBarText->setText(message);
        m_statusBarText->resize(m_statusBarText->sizeHint());
        m_statusBarText->setMaximumWidth(view->width() - verticalScrollSize);

        QPoint position;
        position.setY(view->height() - horizontalScrollSize - m_statusBarText->height());

        QRect statusRect = QRect(view->mapToGlobal(QPoint(0, position.y())), m_statusBarText->size());

        if (statusRect.contains(QCursor::pos()))
            position.setY(position.y() - m_statusBarText->height());

        m_statusBarText->move(view->mapToGlobal(position));
        m_statusBarText->show();
    }
}

void StatusBarMessage::clearMessage()
{
    if (p_QupZilla->statusBar()->isVisible())
        p_QupZilla->statusBar()->showMessage("");
    else
        m_statusBarText->hide();
}
