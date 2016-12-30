/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2014-2016 David Rosca <nowrep@gmail.com>
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
    void updateIcon();

signals:
    void resized();

private slots:
    void showLoadingAnimation();
    void hideLoadingAnimation();

    void updateAudioIcon(bool recentlyAudible);
    void updateAnimationFrame();

private:
    void show();
    void hide();
    bool shouldBeVisible() const;

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);

    WebTab* m_tab;
    QTimer* m_updateTimer;
    QTimer* m_hideTimer;
    QPixmap m_sitePixmap;
    int m_currentFrame;
    bool m_animationRunning;
    bool m_audioIconDisplayed;

    struct Data {
        int framesCount;
        QPixmap animationPixmap;
        QPixmap audioPlayingPixmap;
        QPixmap audioMutedPixmap;
    };
    static Data *s_data;
};

#endif // TABICON_H
