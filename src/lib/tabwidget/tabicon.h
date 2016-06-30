/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014  David Rosca <nowrep@gmail.com>
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
#ifndef TABICON_H
#define TABICON_H

#include <QWidget>
#include <QImage>

#include "qzcommon.h"

class QTimer;

class WebTab;

class QUPZILLA_EXPORT TabIcon : public QWidget
{
    Q_OBJECT

public:
    explicit TabIcon(QWidget* parent = 0);

    void setWebTab(WebTab* tab);
    void setIcon(const QIcon &icon);
    void updateAudioIcon(bool recentlyAudible);

private slots:
    void showIcon();
    void showLoadingAnimation();
    void hideLoadingAnimation();

    void updateAnimationFrame();

private:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);

    WebTab* m_tab;
    QTimer* m_updateTimer;

    QPixmap m_sitePixmap;
    QPixmap m_animationPixmap;
    QPixmap m_audioPlayingPixmap;
    QPixmap m_audioMutedPixmap;
    int m_currentFrame;
    int m_framesCount;
    bool m_animationRunning;
    bool m_audioIconDisplayed;
};

#endif // TABICON_H
