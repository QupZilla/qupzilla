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
#include "locationcompleterview.h"
#include "locationcompletermodel.h"
#include "locationcompleterdelegate.h"
#include "toolbutton.h"
#include "iconprovider.h"
#include "mainapplication.h"
#include "searchenginesmanager.h"
#include "loadrequest.h"

#include <QKeyEvent>
#include <QApplication>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

LocationCompleterView::LocationCompleterView()
    : QWidget(nullptr)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_X11NetWmWindowTypeCombo);

    if (qApp->platformName() == QL1S("xcb")) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    } else {
        setWindowFlags(Qt::Popup);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new QListView(this);
    layout->addWidget(m_view);

    m_view->setUniformItemSizes(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);

    m_view->setMouseTracking(true);
    qApp->installEventFilter(this);

    m_delegate = new LocationCompleterDelegate(this);
    m_view->setItemDelegate(m_delegate);

    QFrame *searchFrame = new QFrame(this);
    searchFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(10, 4, 4, 4);

    ToolButton *searchSettingsButton = new ToolButton(this);
    searchSettingsButton->setIcon(IconProvider::settingsIcon());
    searchSettingsButton->setToolTip(tr("Manage Search Engines"));
    searchSettingsButton->setAutoRaise(true);
    searchSettingsButton->setIconSize(QSize(16, 16));
    connect(searchSettingsButton, &ToolButton::clicked, this, &LocationCompleterView::searchEnginesDialogRequested);

    QLabel *searchLabel = new QLabel(tr("Search with:"));
    m_searchEnginesLayout = new QHBoxLayout();

    setupSearchEngines();
    connect(mApp->searchEnginesManager(), &SearchEnginesManager::enginesChanged, this, &LocationCompleterView::setupSearchEngines);

    searchLayout->addWidget(searchLabel);
    searchLayout->addLayout(m_searchEnginesLayout);
    searchLayout->addStretch();
    searchLayout->addWidget(searchSettingsButton);

    layout->addWidget(searchFrame);
}

QAbstractItemModel *LocationCompleterView::model() const
{
    return m_view->model();
}

void LocationCompleterView::setModel(QAbstractItemModel *model)
{
    m_view->setModel(model);
}

QModelIndex LocationCompleterView::currentIndex() const
{
    return m_view->currentIndex();
}

void LocationCompleterView::setCurrentIndex(const QModelIndex &index)
{
    m_view->setCurrentIndex(index);
}

QItemSelectionModel *LocationCompleterView::selectionModel() const
{
    return m_view->selectionModel();
}

void LocationCompleterView::setOriginalText(const QString &originalText)
{
    m_originalText = originalText;
    m_delegate->setOriginalText(originalText);
}

void LocationCompleterView::adjustSize()
{
    const int maxItemsCount = 12;
    const int newHeight = m_view->sizeHintForRow(0) * qMin(maxItemsCount, model()->rowCount()) + 2 * m_view->frameWidth();

    if (!m_resizeTimer) {
        m_resizeTimer = new QTimer(this);
        m_resizeTimer->setInterval(200);
        connect(m_resizeTimer, &QTimer::timeout, this, [this]() {
            if (m_resizeHeight > 0) {
                m_view->setFixedHeight(m_resizeHeight);
                setFixedHeight(sizeHint().height());
            }
            m_resizeHeight = -1;
        });
    }

    if (!m_forceResize) {
        if (newHeight == m_resizeHeight) {
            return;
        } else if (newHeight == m_view->height()) {
            m_resizeHeight = -1;
            return;
        } else if (newHeight < m_view->height()) {
            m_resizeHeight = newHeight;
            m_resizeTimer->start();
            return;
        }
    }

    m_resizeHeight = -1;
    m_forceResize = false;
    m_view->setFixedHeight(newHeight);
    setFixedHeight(sizeHint().height());
}

bool LocationCompleterView::eventFilter(QObject* object, QEvent* event)
{
    // Event filter based on QCompleter::eventFilter from qcompleter.cpp

    if (object == this || object == m_view || !isVisible()) {
        return false;
    }

    if (object == m_view->viewport()) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            QModelIndex idx = m_view->indexAt(e->pos());
            if (!idx.isValid()) {
                return false;
            }

            Qt::MouseButton button = e->button();
            Qt::KeyboardModifiers modifiers = e->modifiers();

            if (button == Qt::LeftButton && modifiers == Qt::NoModifier) {
                emit indexActivated(idx);
                return true;
            }

            if (button == Qt::MiddleButton || (button == Qt::LeftButton && modifiers == Qt::ControlModifier)) {
                emit indexCtrlActivated(idx);
                return true;
            }

            if (button == Qt::LeftButton && modifiers == Qt::ShiftModifier) {
                emit indexShiftActivated(idx);
                return true;
            }
        }
        return false;
    }

    switch (event->type()) {
    case QEvent::KeyPress: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        const QModelIndex idx = m_view->currentIndex();
        const QModelIndex visitSearchIdx = model()->index(0, 0).data(LocationCompleterModel::VisitSearchItemRole).toBool() ? model()->index(0, 0) : QModelIndex();

        if ((keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) && m_view->currentIndex() != idx) {
            m_view->setCurrentIndex(idx);
        }

        switch (keyEvent->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!idx.isValid()) {
                break;
            }

            if (modifiers == Qt::NoModifier || modifiers == Qt::KeypadModifier) {
                emit indexActivated(idx);
                return true;
            }

            if (modifiers == Qt::ControlModifier) {
                emit indexCtrlActivated(idx);
                return true;
            }

            if (modifiers == Qt::ShiftModifier) {
                emit indexShiftActivated(idx);
                return true;
            }
            break;

        case Qt::Key_End:
            if (modifiers & Qt::ControlModifier) {
                m_view->setCurrentIndex(model()->index(model()->rowCount() - 1, 0));
                return true;
            } else {
                close();
            }
            break;

        case Qt::Key_Home:
            if (modifiers & Qt::ControlModifier) {
                m_view->setCurrentIndex(model()->index(0, 0));
                m_view->scrollToTop();
                return true;
            } else {
                close();
            }
            break;

        case Qt::Key_Escape:
            close();
            return true;

        case Qt::Key_F4:
            if (modifiers == Qt::AltModifier)  {
                close();
                return false;
            }
            break;

        case Qt::Key_Tab:
        case Qt::Key_Backtab: {
            const bool isShift = keyEvent->modifiers() == Qt::ShiftModifier;
            if (keyEvent->modifiers() != Qt::NoModifier && !isShift) {
                return false;
            }
            bool isBack = keyEvent->key() == Qt::Key_Backtab;
            if (keyEvent->key() == Qt::Key_Tab && isShift) {
                isBack = true;
            }
            QKeyEvent ev(QKeyEvent::KeyPress, isBack ? Qt::Key_Up : Qt::Key_Down, Qt::NoModifier);
            QApplication::sendEvent(focusProxy(), &ev);
            return true;
        }

        case Qt::Key_Up:
        case Qt::Key_PageUp: {
            if (keyEvent->modifiers() != Qt::NoModifier) {
                return false;
            }
            const int step = keyEvent->key() == Qt::Key_PageUp ? 5 : 1;
            if (!idx.isValid() || idx == visitSearchIdx) {
                int rowCount = model()->rowCount();
                QModelIndex lastIndex = model()->index(rowCount - 1, 0);
                m_view->setCurrentIndex(lastIndex);
            } else if (idx.row() == 0) {
                m_view->setCurrentIndex(QModelIndex());
            } else {
                m_view->setCurrentIndex(model()->index(qMax(0, idx.row() - step), 0));
            }
            return true;
        }

        case Qt::Key_Down:
        case Qt::Key_PageDown: {
            if (keyEvent->modifiers() != Qt::NoModifier) {
                return false;
            }
            const int step = keyEvent->key() == Qt::Key_PageDown ? 5 : 1;
            if (!idx.isValid()) {
                QModelIndex firstIndex = model()->index(0, 0);
                m_view->setCurrentIndex(firstIndex);
            } else if (idx != visitSearchIdx && idx.row() == model()->rowCount() - 1) {
                m_view->setCurrentIndex(visitSearchIdx);
                m_view->scrollToTop();
            } else {
                m_view->setCurrentIndex(model()->index(qMin(model()->rowCount() - 1, idx.row() + step), 0));
            }
            return true;
        }

        case Qt::Key_Delete:
            if (idx != visitSearchIdx && m_view->viewport()->rect().contains(m_view->visualRect(idx))) {
                emit indexDeleteRequested(idx);
                return true;
            }
            break;

        case Qt::Key_Shift:
            m_delegate->setForceVisitItem(true);
            m_view->viewport()->update();
            return true;
        } // switch (keyEvent->key())

        if (focusProxy()) {
            (static_cast<QObject*>(focusProxy()))->event(keyEvent);
        }
        return true;
    }

    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        switch (keyEvent->key()) {
        case Qt::Key_Shift:
            m_delegate->setForceVisitItem(false);
            m_view->viewport()->update();
            return true;
        }
        break;
    }

    case QEvent::Wheel:
    case QEvent::MouseButtonPress:
        if (!underMouse()) {
            close();
            return false;
        }
        break;

    case QEvent::FocusOut: {
        QFocusEvent *focusEvent = static_cast<QFocusEvent*>(event);
        if (focusEvent->reason() != Qt::PopupFocusReason && focusEvent->reason() != Qt::MouseFocusReason) {
            close();
        }
        break;
    }

    case QEvent::Move:
    case QEvent::Resize: {
        QWidget *w = qobject_cast<QWidget*>(object);
        if (w && w->isWindow() && focusProxy() && w == focusProxy()->window()) {
            close();
        }
        break;
    }

    default:
        break;
    } // switch (event->type())

    return false;
}

void LocationCompleterView::close()
{
    hide();
    m_view->verticalScrollBar()->setValue(0);
    m_delegate->setForceVisitItem(false);
    m_forceResize = true;

    emit closed();
}

void LocationCompleterView::setupSearchEngines()
{
    while (m_searchEnginesLayout->count() != 0) {
        delete m_searchEnginesLayout->takeAt(0);
    }

    const auto engines = mApp->searchEnginesManager()->allEngines();
    for (const SearchEngine &engine : engines) {
        ToolButton *button = new ToolButton(this);
        button->setIcon(engine.icon);
        button->setToolTip(engine.name);
        button->setAutoRaise(true);
        button->setIconSize(QSize(16, 16));
        connect(button, &ToolButton::clicked, this, [=]() {
            emit loadRequested(mApp->searchEnginesManager()->searchResult(engine, m_originalText));
        });
        m_searchEnginesLayout->addWidget(button);
    }
}
