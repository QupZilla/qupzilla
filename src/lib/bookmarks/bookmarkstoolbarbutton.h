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
#ifndef BOOKMARKSTOOLBARBUTTON_H
#define BOOKMARKSTOOLBARBUTTON_H

#include <QPushButton>

#include "qz_namespace.h"

class Menu;
class QupZilla;
class BookmarkItem;

class QT_QUPZILLA_EXPORT BookmarksToolbarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit BookmarksToolbarButton(BookmarkItem* bookmark, QWidget* parent = 0);

    BookmarkItem* bookmark() const;
    void setMainWindow(QupZilla* window);

    bool showOnlyIcon() const;
    void setShowOnlyIcon(bool show);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private slots:
    void createMenu();

    void menuMiddleClicked(Menu* menu);
    void bookmarkActivated(BookmarkItem* item = 0);
    void bookmarkCtrlActivated(BookmarkItem* item = 0);
    void bookmarkShiftActivated(BookmarkItem* item = 0);

    void openFolder(BookmarkItem* item);
    void openBookmark(BookmarkItem* item);
    void openBookmarkInNewTab(BookmarkItem* item);
    void openBookmarkInNewWindow(BookmarkItem* item);

private:
    void init();
    QString createTooltip() const;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

    BookmarkItem* m_bookmark;
    QupZilla* m_window;

    Qt::MouseButtons m_buttons;
    Qt::KeyboardModifiers m_modifiers;
    bool m_showOnlyIcon;
    int m_padding;

};

#endif // BOOKMARKSTOOLBARBUTTON_H
