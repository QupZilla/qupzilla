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
#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include <QProxyStyle>

#include "qzcommon.h"

class QUPZILLA_EXPORT ProxyStyle : public QProxyStyle
{
public:
    explicit ProxyStyle();

    int styleHint(StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const;
    int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

    QString name() const;

private:
    mutable int m_TabBarTabHSpace;
};

#endif // PROXYSTYLE_H
