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
#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include <QTimer>

#include "qzcommon.h"

class QUPZILLA_EXPORT ToolButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY(QSize fixedsize READ size WRITE setFixedSize)
    Q_PROPERTY(int fixedwidth READ width WRITE setFixedWidth)
    Q_PROPERTY(int fixedheight READ height WRITE setFixedHeight)
    Q_PROPERTY(QImage multiIcon READ multiIcon WRITE setMultiIcon)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString themeIcon READ themeIcon WRITE setThemeIcon)
    Q_PROPERTY(QIcon fallbackIcon READ fallbackIcon WRITE setFallbackIcon)

public:
    explicit ToolButton(QWidget* parent = 0);

    // MultiIcon - Image containing pixmaps for all button states
    QImage multiIcon() const;
    void setMultiIcon(const QImage &image);

    // ThemeIcon - Standard QToolButton with theme icon
    QString themeIcon() const;
    void setThemeIcon(const QString &icon);

    // FallbackIcon - In case theme doesn't contain ThemeIcon
    QIcon fallbackIcon() const;
    void setFallbackIcon(const QIcon &fallbackIcon);

    // Icon - Standard QToolButton with icon
    QIcon icon() const;
    void setIcon(const QIcon &icon);

    // Menu - Menu is handled in ToolButton and is not passed to QToolButton
    // There won't be menu indicator shown in the button
    // QToolButton::MenuButtonPopup is not supported
    QMenu* menu() const;
    void setMenu(QMenu* menu);

    // Align the right corner of menu to the right corner of button
    bool showMenuInside() const;
    void setShowMenuInside(bool enable);

    // Show button menu on right click
    bool showMenuOnRightClick() const;
    void setShowMenuOnRightClick(bool enable);

    // Set the button to look as it was in toolbar
    // (it now only sets the correct icon size)
    bool toolbarButtonLook() const;
    void setToolbarButtonLook(bool enable);

signals:
    void middleMouseClicked();
    void controlClicked();
    void doubleClicked();

    // It is needed to use these signals with ShowMenuInside
    void aboutToShowMenu();
    void aboutToHideMenu();

private slots:
    void menuAboutToHide();
    void showMenu();

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void contextMenuEvent(QContextMenuEvent *e);
    void paintEvent(QPaintEvent* e);

private:
    QImage m_multiIcon;
    QString m_themeIcon;
    QTimer m_pressTimer;
    QMenu* m_menu;

    enum Options {
        MultiIconOption = 1,
        ShowMenuInsideOption = 2,
        ToolBarLookOption = 4,
        ShowMenuOnRightClick = 8
    };
    Q_DECLARE_FLAGS(OptionsFlags, Options)
    OptionsFlags m_options;
};


#endif // TOOLBUTTON_H
