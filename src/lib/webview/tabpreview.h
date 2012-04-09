/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  Alexander Samilovskih <alexsamilovskih@gmail.com>
*                          David Rosca <nowrep@gmail.com>
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
#ifndef TABPREVIEW_H
#define TABPREVIEW_H

#include <QFrame>

class QupZilla;
class WebTab;
class QLabel;
class QPropertyAnimation;
class QParallelAnimationGroup;
class QGraphicsOpacityEffect;

class TabPreview : public QFrame
{
    Q_OBJECT
public:
    explicit TabPreview(QupZilla* mainClass, QWidget* parent);

    void setWebTab(WebTab* webTab, bool noPixmap);
    void showOnRect(const QRect &rect);

    void setAnimationsEnabled(bool enabled);

public slots:
    void hideAnimated();
    void hide();

    void show();

protected:
    void paintEvent(QPaintEvent* pe);

private:
    void showAnimated();
    void setFinishingGeometry(const QRect &oldGeometry, const QRect &newGeometry);
    QPoint calculatePosition(const QRect &tabRect, const QSize &previewSize);

    QupZilla* p_QupZilla;
    QLabel* m_pixmap;
    QLabel* m_title;

    bool m_animationsEnabled;
    QPropertyAnimation* m_animation;
    QPropertyAnimation* m_opacityAnimation;
    QGraphicsOpacityEffect* m_opacityEffect;
};

#endif // TABPREVIEW_H
