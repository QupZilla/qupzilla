#ifndef LOCATIONBAR_H
#define LOCATIONBAR_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QLabel>
#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItem>
#include <QUrl>
#include <QSettings>
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QTableView>
#include "lineedit.h"

class QupZilla;
class LineEdit;
class LocationCompleter;
class ClickableLabel;
class BookmarksModel;

class LocationBar : public LineEdit
{
    Q_OBJECT;
public:
    explicit LocationBar(QupZilla* mainClass, QWidget *parent = 0);
    ~LocationBar();
    static QIcon icon(const QUrl &url);

    void loadSettings();

public slots:
    void checkRss();
    void showUrl(const QUrl &url, bool empty = true);
    void siteIconChanged();
    void setPrivacy(bool state);
    void addRss();
    void textEdit();
    void showPopup();
    void bookmarkIconClicked();
    void checkBookmark();
    void showSiteInfo();

private:
    void focusOutEvent(QFocusEvent* e);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void dropEvent(QDropEvent *event);

    void showGoButton();
    void hideGoButton();

    ClickableLabel* m_bookmarkButton;
    ClickableLabel* m_goButton;
    QToolButton* m_siteIcon;
    QToolButton* m_rssIcon;
    QMenu* m_rssMenu;

    bool m_selectAllOnDoubleClick;
    bool m_addComWithCtrl;
    bool m_addCountryWithAlt;
    QupZilla* p_QupZilla;
    LocationCompleter* m_locationCompleter;
    BookmarksModel* m_bookmarksModel;

    bool m_rssIconVisible;
};

#endif // LOCATIONBAR_H
