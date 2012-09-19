/*
 *   Bespin library for Qt style, KWin decoration and everythng else
 *   Copyright 2007-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef COLORS_H
#define COLORS_H

class QWidget;
#include <QColor>
#include <QPalette>

// namespace Bespin {
namespace Colors
{

const QColor &bg(const QPalette &pal, const QWidget* w);
int contrast(const QColor &a, const QColor &b);
QPalette::ColorRole counterRole(QPalette::ColorRole role);
bool counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to,
                 QPalette::ColorRole defFrom = QPalette::WindowText,
                 QPalette::ColorRole defTo = QPalette::Window);
QColor emphasize(const QColor &c, int value = 10);
bool haveContrast(const QColor &a, const QColor &b);
QColor light(const QColor &c, int value);
QColor mid(const QColor &oc1, const QColor &c2, int w1 = 1, int w2 = 1);
int value(const QColor &c);

}
// }

#endif //COLORS_H
