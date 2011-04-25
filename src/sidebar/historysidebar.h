#ifndef HISTORYSIDEBAR_H
#define HISTORYSIDEBAR_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QPointer>
#include <QShortcut>

#include "historymodel.h"

namespace Ui {
    class HistorySideBar;
}

class QupZilla;
class HistorySideBar : public QWidget
{
    Q_OBJECT

public:
    explicit HistorySideBar(QupZilla* mainClass, QWidget* parent = 0);
    ~HistorySideBar();

public slots:
    void refreshTable();

private slots:
    void search();
    void itemDoubleClicked(QTreeWidgetItem* item);
    void deleteItem();
    void contextMenuRequested(const QPoint &position);
    void loadInNewTab();
    void itemControlClicked(QTreeWidgetItem* item);

    void historyEntryAdded(const HistoryModel::HistoryEntry &entry);
    void historyEntryDeleted(const HistoryModel::HistoryEntry &entry);

private:
    Ui::HistorySideBar* ui;
    QPointer<QupZilla> p_QupZilla;
    HistoryModel* m_historyModel;
};

#endif // HISTORYSIDEBAR_H
