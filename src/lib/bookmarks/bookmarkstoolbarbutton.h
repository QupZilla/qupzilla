/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2014-2018 David Rosca <nowrep@gmail.com>
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

#include "qzcommon.h"

class Menu;
class BrowserWindow;
class BookmarkItem;

class QUPZILLA_EXPORT BookmarksToolbarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit BookmarksToolbarButton(BookmarkItem* bookmark, QWidget* parent = 0);

    BookmarkItem* bookmark() const;
    void setMainWindow(BrowserWindow* window);

    bool showOnlyIcon() const;
    void setShowOnlyIcon(bool show);

    bool showOnlyText() const;
    void setShowOnlyText(bool show);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void createMenu();

    void menuAboutToShow();
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

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* event);

    BookmarkItem* m_bookmark;
    BrowserWindow* m_window;

    bool m_showOnlyIcon;
    bool m_showOnlyText;
};

#endif // BOOKMARKSTOOLBARBUTTON_H
