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
#include "statusbarmessage.h"
#include "qupzilla.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "squeezelabelv1.h"
#include "mainapplication.h"
#include "webpage.h"
#include "proxystyle.h"
#include "qztools.h"

#include <QStyleOptionFrame>
#include <QStatusBar>
#include <QToolTip>
#include <QStylePainter>
#include <QWebFrame>
#include <QTimer>

TipLabel::TipLabel(QWidget* parent)
    : SqueezeLabelV1(parent)
{
    setWindowFlags(Qt::ToolTip);
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    setFrameStyle(QFrame::NoFrame);
    setMargin(3);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hide()));
}

void TipLabel::show(QWidget* widget)
{
    m_timer->stop();

    widget->installEventFilter(this);
    SqueezeLabelV1::show();
}

void TipLabel::hideDelayed()
{
    m_timer->start();
}

void TipLabel::resizeEvent(QResizeEvent* ev)
{
    SqueezeLabelV1::resizeEvent(ev);

    // Oxygen is setting rounded corners only for top-level tooltips
    if (mApp->proxyStyle() && mApp->proxyStyle()->name() == QLatin1String("oxygen")) {
        setMask(QzTools::roundedRect(rect(), 4));
    }
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
    case QEvent::WindowDeactivate:
    case QEvent::Wheel:
        hide();
        break;

    default:
        break;
    }

    return false;
}

StatusBarMessage::StatusBarMessage(QupZilla* mainClass)
    : p_QupZilla(mainClass)
    , m_statusBarText(new TipLabel(mainClass))
{
}

void StatusBarMessage::showMessage(const QString &message)
{
    if (p_QupZilla->statusBar()->isVisible()) {
        p_QupZilla->statusBar()->showMessage(message);
    }
#ifdef Q_OS_WIN
    else if (mApp->activeWindow() == p_QupZilla) {
#else
    else {
#endif
        WebView* view = p_QupZilla->weView();
        QWebFrame* mainFrame = view->page()->mainFrame();

        int horizontalScrollSize = 0;
        int verticalScrollSize = 0;
        const int scrollbarSize = p_QupZilla->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

        if (mainFrame->scrollBarMaximum(Qt::Horizontal)) {
            horizontalScrollSize = scrollbarSize;
        }
        if (mainFrame->scrollBarMaximum(Qt::Vertical)) {
            verticalScrollSize = scrollbarSize;
        }

        m_statusBarText->setText(message);
        m_statusBarText->setMaximumWidth(view->width() - verticalScrollSize);
        m_statusBarText->resize(m_statusBarText->sizeHint());

        QPoint position(0, view->height() - horizontalScrollSize - m_statusBarText->height());
        const QRect statusRect = QRect(view->mapToGlobal(QPoint(0, position.y())), m_statusBarText->size());

        if (statusRect.contains(QCursor::pos())) {
            position.setY(position.y() - m_statusBarText->height());
        }

        m_statusBarText->move(view->mapToGlobal(position));
        m_statusBarText->show(view);
    }
}

void StatusBarMessage::clearMessage()
{
    if (p_QupZilla->statusBar()->isVisible()) {
        p_QupZilla->statusBar()->showMessage(QString());
    }
    else {
        m_statusBarText->hideDelayed();
    }
}
