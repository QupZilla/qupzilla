/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H

#include <QDialog>
#include <QStyledItemDelegate>

#include "qzcommon.h"

class QIcon;

namespace Ui
{
class IconChooser;
}

class QUPZILLA_EXPORT IconChooser : public QDialog
{
    Q_OBJECT

public:
    explicit IconChooser(QWidget* parent = 0);
    ~IconChooser();

    QIcon getIcon();

private slots:
    void chooseFile();
    void searchIcon(const QString &string);

private:
    Ui::IconChooser* ui;
};

class QUPZILLA_EXPORT IconChooserDelegate : public QStyledItemDelegate
{
public:
    explicit IconChooserDelegate(QWidget* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ICONCHOOSER_H
