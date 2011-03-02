#ifndef AUTOFILLMANAGER_H
#define AUTOFILLMANAGER_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QSqlQuery>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>

namespace Ui {
    class AutoFillManager;
}

class AutoFillManager : public QDialog
{
    Q_OBJECT

public:
    explicit AutoFillManager(QWidget *parent = 0);
    ~AutoFillManager();

    void showExceptions();

private slots:
    void loadPasswords();

    void removePass();
    void removeAllPass();
    void editPass();

    void removeExcept();
    void removeAllExcept();

private:
    Ui::AutoFillManager *ui;
};

#endif // AUTOFILLMANAGER_H
