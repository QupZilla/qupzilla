/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2016 David Rosca <nowrep@gmail.com>
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

#ifndef WEBSCROLLBAR_H
#define WEBSCROLLBAR_H

#include <QScrollBar>

#include "qzcommon.h"

class WebView;

class WebScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit WebScrollBar(Qt::Orientation orientation, WebView *view);

    int thickness() const;
    void updateValues(const QSize &viewport);

private:
    void performScroll();
    void paintEvent(QPaintEvent *ev) override;

    WebView *m_view;
    bool m_blockScrolling = false;
};

#endif // WEBSCROLLBAR_H
