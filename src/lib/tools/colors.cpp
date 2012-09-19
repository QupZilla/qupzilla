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

#include "colors.h"
#define CLAMP(x,l,u) (x) < (l) ? (l) :\
    (x) > (u) ? (u) :\
    (x)
#include <QWidget>
#include <QApplication>

// using namespace Bespin;

const QColor &
Colors::bg(const QPalette &pal, const QWidget* w)
{
    QPalette::ColorRole role;
    if (!w) {
        role = QPalette::Window;
    }
    else if (w->parentWidget()) {
        role = w->parentWidget()->backgroundRole();
    }
    else {
        role = w->backgroundRole();
    }

//     if (pal.brush(role).style() > 1)
    return pal.color(role);
//     return QApplication::palette().color(role);
}

int
Colors::contrast(const QColor &a, const QColor &b)
{
    int ar, ag, ab, br, bg, bb;
    a.getRgb(&ar, &ag, &ab);
    b.getRgb(&br, &bg, &bb);

    int diff = 299 * (ar - br) + 587 * (ag - bg) + 114 * (ab - bb);
    diff = (diff < 0) ? -diff : 90 * diff / 100;
    int perc = diff / 2550;

    diff = qMax(ar, br) + qMax(ag, bg) + qMax(ab, bb)
           - (qMin(ar, br) + qMin(ag, bg) + qMin(ab, bb));

    perc += diff / 765;
    perc /= 2;

    return perc;
}

QPalette::ColorRole
Colors::counterRole(QPalette::ColorRole role)
{
    switch (role) {
    case QPalette::ButtonText: //8
        return QPalette::Button;
    case QPalette::WindowText: //0
        return QPalette::Window;
    case QPalette::HighlightedText: //13
        return QPalette::Highlight;
    case QPalette::Window: //10
        return QPalette::WindowText;
    case QPalette::Base: //9
        return QPalette::Text;
    case QPalette::Text: //6
        return QPalette::Base;
    case QPalette::Highlight: //12
        return QPalette::HighlightedText;
    case QPalette::Button: //1
        return QPalette::ButtonText;
    default:
        return QPalette::Window;
    }
}

bool
Colors::counterRole(QPalette::ColorRole &from, QPalette::ColorRole &to, QPalette::ColorRole defFrom,
                    QPalette::ColorRole defTo)
{
    switch (from) {
    case QPalette::WindowText: //0
        to = QPalette::Window;
        break;
    case QPalette::Window: //10
        to = QPalette::WindowText;
        break;
    case QPalette::Base: //9
        to = QPalette::Text;
        break;
    case QPalette::Text: //6
        to = QPalette::Base;
        break;
    case QPalette::Button: //1
        to = QPalette::ButtonText;
        break;
    case QPalette::ButtonText: //8
        to = QPalette::Button;
        break;
    case QPalette::Highlight: //12
        to = QPalette::HighlightedText;
        break;
    case QPalette::HighlightedText: //13
        to = QPalette::Highlight;
        break;
    default:
        from = defFrom;
        to = defTo;
        return false;
    }
    return true;
}

QColor
Colors::emphasize(const QColor &c, int value)
{
    int h, s, v, a;
    QColor ret;
    c.getHsv(&h, &s, &v, &a);
    if (v < 75 + value) {
        ret.setHsv(h, s, CLAMP(85 + value, 85, 255), a);
        return ret;
    }
    if (v > 200) {
        if (s > 30) {
            h -= 5;
            if (h < 0) {
                h = 360 + h;
            }
            s = (s << 3) / 9;
            v += value;
            ret.setHsv(h, CLAMP(s, 30, 255), CLAMP(v, 0, 255), a);
            return ret;
        }
        if (v > 230) {
            ret.setHsv(h, s, CLAMP(v - value, 0, 255), a);
            return ret;
        }
    }
    if (v > 128) {
        ret.setHsv(h, s, CLAMP(v + value, 0, 255), a);
    }
    else {
        ret.setHsv(h, s, CLAMP(v - value, 0, 255), a);
    }
    return ret;
}

bool
Colors::haveContrast(const QColor &a, const QColor &b)
{
    int ar, ag, ab, br, bg, bb;
    a.getRgb(&ar, &ag, &ab);
    b.getRgb(&br, &bg, &bb);

    int diff = (299 * (ar - br) + 587 * (ag - bg) + 114 * (ab - bb));

    if (qAbs(diff) < 91001) {
        return false;
    }

    diff = qMax(ar, br) + qMax(ag, bg) + qMax(ab, bb)
           - (qMin(ar, br) + qMin(ag, bg) + qMin(ab, bb));

    return (diff > 300);
}

QColor
Colors::light(const QColor &c, int value)
{
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);
    QColor ret;
    if (v < 255 - value) {
        ret.setHsv(h, s, CLAMP(v + value, 0, 255), a); //value could be negative
        return ret;
    }
    // psychovisual uplightning, i.e. shift hue and lower saturation
    if (s > 30) {
        h -= (value * 5 / 20);
        if (h < 0) {
            h = 400 + h;
        }
        s = CLAMP((s << 3) / 9, 30, 255);
        ret.setHsv(h, s, 255, a);
        return ret;
    }
    else { // hue shifting has no sense, half saturation (btw, white won't get brighter :)
        ret.setHsv(h, s >> 1, 255, a);
    }
    return ret;
}

QColor
Colors::mid(const QColor &c1, const QColor &c2, int w1, int w2)
{
    int sum = (w1 + w2);
    if (!sum) {
        return Qt::black;
    }

    int r, g, b, a;
#if 0
    QColor c1 = oc1;
    b = value(c1);
    if (b < 70) {
        c1.getHsv(&r, &g, &b, &a);
        c1.setHsv(r, g, 70, a);
    }
#endif
    r = (w1 * c1.red() + w2 * c2.red()) / sum;
    r = CLAMP(r, 0, 255);
    g = (w1 * c1.green() + w2 * c2.green()) / sum;
    g = CLAMP(g, 0, 255);
    b = (w1 * c1.blue() + w2 * c2.blue()) / sum;
    b = CLAMP(b, 0, 255);
    a = (w1 * c1.alpha() + w2 * c2.alpha()) / sum;
    a = CLAMP(a, 0, 255);
    return QColor(r, g, b, a);
}

int
Colors::value(const QColor &c)
{
    int v = c.red();
    if (c.green() > v) {
        v = c.green();
    }
    if (c.blue() > v) {
        v = c.blue();
    }
    return v;
}
