/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionToolButton>

class ToolButton : public QToolButton
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

    void setData(const QVariant& data);
    QVariant data();

    void setMultiIcon(const QPixmap &icon);
    QPixmap pixmap() { return m_normalIcon; }

    void setThemeIcon(const QString &icon);
    QString themeIcon() { return m_themeIcon; }

    void setIcon(const QIcon &icon);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *e);

    bool m_usingMultiIcon;

    QPixmap m_normalIcon;
    QPixmap m_hoverIcon;
    QPixmap m_activeIcon;
    QPixmap m_disabledIcon;

    QString m_themeIcon;
    QVariant m_data;

};

#endif // TOOLBUTTON_H
