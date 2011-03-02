#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QTreeWidgetItem>
#include <QPointer>

namespace Ui {
    class HistoryManager;
}

class QupZilla;
class HistoryManager : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryManager(QupZilla* mainClass, QWidget *parent = 0);
    ~HistoryManager();

    void setMainWindow(QupZilla *window);

public slots:
    void refreshTable();

private slots:
    void itemDoubleClicked(QTreeWidgetItem* item);
    void deleteItem();
    void clearHistory();
    void search();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);

private:
    QupZilla* getQupZilla();
    Ui::HistoryManager *ui;
    QPointer<QupZilla> p_QupZilla;
};

#endif // HISTORYMANAGER_H
