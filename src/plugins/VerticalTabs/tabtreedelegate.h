/* ============================================================
* VerticalTabs plugin for QupZilla
* Copyright (C) 2018 David Rosca <nowrep@gmail.com>
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
#pragma once

#include <QAbstractButton>
#include <QStyledItemDelegate>

class QPushButton;

class TabTreeView;
class LoadingAnimator;

class TabTreeCloseButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int showOnNormal READ showOnNormal WRITE setShowOnNormal)
    Q_PROPERTY(int showOnHovered READ showOnHovered WRITE setShowOnHovered)
    Q_PROPERTY(int showOnSelected READ showOnSelected WRITE setShowOnSelected)

public:
    explicit TabTreeCloseButton(QWidget *parent = nullptr);

    int showOnNormal() const;
    void setShowOnNormal(int show);

    int showOnHovered() const;
    void setShowOnHovered(int show);

    int showOnSelected() const;
    void setShowOnSelected(int show);

    bool isVisible(bool hovered, bool selected) const;

private:
    void paintEvent(QPaintEvent *event) override;

    int m_showOnNormal = 0;
    int m_showOnHovered = 1;
    int m_showOnSelected = 1;
};

class TabTreeDelegate : public QStyledItemDelegate
{
public:
    explicit TabTreeDelegate(TabTreeView *view);

    QRect expandButtonRect(const QModelIndex &index) const;
    QRect audioButtonRect(const QModelIndex &index) const;
    QRect closeButtonRect(const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    TabTreeView *m_view;
    LoadingAnimator *m_loadingAnimator;
    TabTreeCloseButton *m_closeButton;
    int m_padding;
    int m_indentation;
};
