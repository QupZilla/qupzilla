#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H
#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QMenu>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QMouseEvent>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

namespace Ui {
    class BookmarksWidget;
}

class BookmarksModel;
class BookmarksWidget : public QMenu
{
    Q_OBJECT
public:
    explicit BookmarksWidget(int bookmarkId, QWidget *parent = 0);
    ~BookmarksWidget();
    void showAt(QWidget* _parent);

signals:
    void bookmarkDeleted();

public slots:

private slots:
    void removeBookmark();
    void saveBookmark();

private:
    void loadBookmark();

    Ui::BookmarksWidget* ui;
    int m_bookmarkId;
    BookmarksModel* m_bookmarksModel;
};

#endif // BOOKMARKSWIDGET_H
