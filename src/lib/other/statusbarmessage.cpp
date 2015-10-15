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
#include "browserwindow.h"
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
    if (mApp->styleName() == QLatin1String("oxygen")) {
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

    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        if (o == this)
            hide();
        break;

    default:
        break;
    }

    return false;
}

StatusBarMessage::StatusBarMessage(BrowserWindow* window)
    : m_window(window)
    , m_statusBarText(new TipLabel(window))
{
}

void StatusBarMessage::showMessage(const QString &message)
{
    if (m_window->statusBar()->isVisible()) {
        const static QChar LRE(0x202a);
        m_window->statusBar()->showMessage(message.isRightToLeft() ? message : (LRE + message));
    }
#ifdef Q_OS_WIN
    else if (mApp->activeWindow() == m_window) {
#else
    else {
#endif
        WebView* view = m_window->weView();
        QWebFrame* mainFrame = view->page()->mainFrame();

        int horizontalScrollSize = 0;
        int verticalScrollSize = 0;
        const int scrollbarSize = m_window->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

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
    if (m_window->statusBar()->isVisible()) {
        m_window->statusBar()->showMessage(QString());
    }
    else {
        m_statusBarText->hideDelayed();
    }
}
