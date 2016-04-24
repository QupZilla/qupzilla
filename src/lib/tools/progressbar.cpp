/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2016 David Rosca <nowrep@gmail.com>
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
#include "progressbar.h"

#include <QStylePainter>
#include <QStyleOptionProgressBarV2>

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent)
    , m_value(0)
    , m_lastPaintedValue(-1)
{
    setMinimumSize(130, 16);
    setMaximumSize(150, 16);
}

void ProgressBar::setValue(int value)
{
    m_value = value;
    if (m_lastPaintedValue != m_value) {
        update();
    }
}

void ProgressBar::initStyleOption(QStyleOptionProgressBar* option)
{
    if (!option) {
        return;
    }

    option->initFrom(this);
    option->minimum = 0;
    option->maximum = 100;
    option->progress = m_value;
    option->textAlignment = Qt::AlignLeft;
    option->textVisible = false;
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    QStylePainter paint(this);

    QStyleOptionProgressBar opt;
    initStyleOption(&opt);

    paint.drawControl(QStyle::CE_ProgressBar, opt);

    m_lastPaintedValue = m_value;
}
