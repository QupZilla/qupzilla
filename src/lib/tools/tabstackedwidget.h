/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
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
#ifndef TABSTACKEDWIDGET_H
#define TABSTACKEDWIDGET_H

#include "qz_namespace.h"

#include <QWidget>

class ComboTabBar;

class QStackedWidget;
class QVBoxLayout;


class QT_QUPZILLA_EXPORT TabStackedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabStackedWidget(QWidget* parent = 0);
    ~TabStackedWidget();

    ComboTabBar* tabBar();
    void setTabBar(ComboTabBar* tb);

    bool isValid(int index);

    bool documentMode() const;
    void setDocumentMode(bool enabled);

    int addTab(QWidget* widget, const QString &label, bool pinned = false);
    int insertTab(int index, QWidget* widget, const QString &label, bool pinned = false);

    QString tabText(int index) const;
    void setTabText(int index, const QString &label);

    QString tabToolTip(int index) const;
    void setTabToolTip(int index, const QString &tip);

    int pinUnPinTab(int index, const QString &title = QString());

    void removeTab(int index);

    void setUpLayout();

    int currentIndex() const;
    QWidget* currentWidget() const;
    QWidget* widget(int index) const;
    int indexOf(QWidget* widget) const;
    int count() const;

public slots:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget* widget);

private:
    QStackedWidget* m_stack;
    ComboTabBar* m_tabBar;
    QVBoxLayout* m_mainLayout;
    bool m_dirtyTabBar;

private slots:
    void tabWasRemoved(int index);
    void showTab(int index);
    void tabWasMoved(int from, int to);

protected:
    bool eventFilter(QObject* obj, QEvent* event);
    void keyPressEvent(QKeyEvent* event);

signals:
    void currentChanged(int index);
    void tabCloseRequested(int index);
};

#endif // TABSTACKEDWIDGET_H
