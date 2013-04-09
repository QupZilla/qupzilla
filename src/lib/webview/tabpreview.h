/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2013  Alexander Samilovskih <alexsamilovskih@gmail.com>
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
#include <QTimeLine>

#ifdef ENABLE_OPACITY_EFFECT
#include <QGraphicsOpacityEffect>
#endif

class QupZilla;
class WebTab;
class QLabel;

class TabPreview : public QFrame
{
    Q_OBJECT
public:
    explicit TabPreview(QupZilla* mainClass, QWidget* parent);

    void setWebTab(WebTab* webTab, bool noPixmap);
    void showOnRect(const QRect &rect);

    int previewIndex();
    void setPreviewIndex(int index);

    void setAnimationsEnabled(bool enabled);

public slots:
    void hideAnimated();
    void hide();

    void show();

private slots:
    void setAnimationFrame(int frame);
#ifdef ENABLE_OPACITY_EFFECT
    void setOpacity(int opacity);
#endif

protected:
    void paintEvent(QPaintEvent* pe);

private:
    void showAnimated();
    void calculateSteps(const QRect &oldGeometry, const QRect &newGeometry);
    QPoint calculatePosition(const QRect &tabRect, const QSize &previewSize);

    QupZilla* p_QupZilla;
    QLabel* m_pixmapLabel;
    QLabel* m_title;

    int m_previewIndex;
    bool m_animationsEnabled;

#ifdef ENABLE_OPACITY_EFFECT
    QTimeLine m_opacityTimeLine;
    QGraphicsOpacityEffect m_opacityEffect;
#endif

    QTimeLine m_animation;
    QRect m_startGeometry;
    qreal m_stepX;
    qreal m_stepY;
    qreal m_stepWidth;
    qreal m_stepHeight;
};

#endif // TABPREVIEW_H
