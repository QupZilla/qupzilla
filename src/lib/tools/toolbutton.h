/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
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
#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include <QVariant>

#include "qzcommon.h"

class QUPZILLA_EXPORT ToolButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)
    Q_PROPERTY(QPixmap multiIcon READ pixmap WRITE setMultiIcon)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString themeIcon READ themeIcon WRITE setThemeIcon)

public:
    explicit ToolButton(QWidget* parent = 0);

    // MultiIcon - Image containing pixmaps for all button states
    QPixmap pixmap() const;
    void setMultiIcon(const QPixmap &icon);

    // ThemeIcon - Standard QToolButton with theme icon
    QString themeIcon() const;
    void setThemeIcon(const QString &icon);

    void setIcon(const QIcon &icon);
    void setMenu(QMenu* menu);

    bool showMenuInside() const;
    void setShowMenuInside(bool inside);

signals:
    void middleMouseClicked();
    void controlClicked();
    void doubleClicked();

    // Emitted when showMenuInside is true
    void aboutToShowMenu();

private slots:
    void menuAboutToHide();

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

private:
    void showMenu();

    bool m_usingMultiIcon;
    bool m_showMenuInside;

    QPixmap m_normalIcon;
    QPixmap m_hoverIcon;
    QPixmap m_activeIcon;
    QPixmap m_disabledIcon;

    QString m_themeIcon;
};

#endif // TOOLBUTTON_H
