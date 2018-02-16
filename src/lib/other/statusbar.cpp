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
#include "statusbar.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include "tabbedwebview.h"
#include "squeezelabelv1.h"
#include "mainapplication.h"
#include "webpage.h"
#include "proxystyle.h"
#include "qztools.h"
#include "abstractbuttoninterface.h"
#include "clickablelabel.h"

#include <QStyleOptionFrame>
#include <QStatusBar>
#include <QToolTip>
#include <QStylePainter>
#include <QTimer>

class StatusBarButton : public ClickableLabel
{
public:
    explicit StatusBarButton(AbstractButtonInterface *button, QWidget *parent = nullptr);

private:
    void clicked();
    void updateIcon();
    void updateToolTip();

    AbstractButtonInterface *m_button;
};

StatusBarButton::StatusBarButton(AbstractButtonInterface *button, QWidget *parent)
    : ClickableLabel(parent)
    , m_button(button)
{
    setFixedSize(16, 16);
    setCursor(Qt::PointingHandCursor);

    updateIcon();
    updateToolTip();
    setVisible(m_button->isVisible());

    connect(this, &ClickableLabel::clicked, this, &StatusBarButton::clicked);
    connect(m_button, &AbstractButtonInterface::iconChanged, this, &StatusBarButton::updateIcon);
    connect(m_button, &AbstractButtonInterface::activeChanged, this, &StatusBarButton::updateIcon);
    connect(m_button, &AbstractButtonInterface::toolTipChanged, this, &StatusBarButton::updateToolTip);
    connect(m_button, &AbstractButtonInterface::badgeTextChanged, this, &StatusBarButton::updateToolTip);
    connect(m_button, &AbstractButtonInterface::visibleChanged, this, &StatusBarButton::setVisible);
}

void StatusBarButton::clicked()
{
    AbstractButtonInterface::ClickController *c = new AbstractButtonInterface::ClickController;
    c->visualParent = this;
    c->popupPosition = [=](const QSize &size) {
        QPoint pos = mapToGlobal(rect().topRight());
        if (QApplication::isRightToLeft()) {
            pos.setX(pos.x() - rect().width());
        } else {
            pos.setX(pos.x() - size.width());
        }
        pos.setY(pos.y() - size.height());
        c->popupOpened = true;
        return pos;
    };
    c->popupClosed = [=]() {
        delete c;
    };
    emit m_button->clicked(c);
    if (!c->popupOpened) {
        c->popupClosed();
    }
}

void StatusBarButton::updateIcon()
{
    const QIcon::Mode mode = m_button->isActive() ? QIcon::Normal : QIcon::Disabled;
    const QImage img = m_button->icon().pixmap(size(), mode).toImage();
    setPixmap(QPixmap::fromImage(img, Qt::MonoOnly));
}

void StatusBarButton::updateToolTip()
{
    QString text = m_button->toolTip();
    if (!m_button->badgeText().isEmpty()) {
        text.append(QSL(" (%1)").arg(m_button->badgeText()));
    }
    setToolTip(text);
}

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

StatusBar::StatusBar(BrowserWindow* window)
    : m_window(window)
    , m_statusBarText(new TipLabel(window))
{
}

void StatusBar::showMessage(const QString &message, int timeout)
{
    if (isVisible()) {
        const static QChar LRE(0x202a);
        QStatusBar::showMessage(message.isRightToLeft() ? message : (LRE + message), timeout);
        return;
    }

    if (mApp->activeWindow() != m_window) {
        return;
    }

    WebView* view = m_window->weView();

    const int verticalScrollSize = view->scrollBarGeometry(Qt::Vertical).width();;
    const int horizontalScrollSize = view->scrollBarGeometry(Qt::Horizontal).height();

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

void StatusBar::clearMessage()
{
    QStatusBar::clearMessage();
    m_statusBarText->hideDelayed();
}

void StatusBar::addButton(AbstractButtonInterface *button)
{
    if (!button || !button->isValid()) {
        return;
    }

    StatusBarButton *widget = new StatusBarButton(button, this);
    widget->setProperty("button-id", button->id());

    WidgetData data;
    data.id = button->id();
    data.widget = widget;
    data.button = button;
    m_widgets[data.id] = data;

    addPermanentWidget(widget);
}

void StatusBar::removeButton(AbstractButtonInterface *button)
{
    if (!button || !m_widgets.contains(button->id())) {
        return;
    }

    delete m_widgets.take(button->id()).widget;
}
