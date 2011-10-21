#ifndef SEARCHENGINESDIALOG_H
#define SEARCHENGINESDIALOG_H

#include <QDialog>

namespace Ui {
    class SearchEnginesDialog;
}

class SearchEnginesManager;
class SearchEnginesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchEnginesDialog(QWidget *parent = 0);
    ~SearchEnginesDialog();

public slots:
    void accept();

private slots:
    void addEngine();
    void removeEngine();
    void editEngine();

    void defaults();

private:
    void reloadEngines();

    Ui::SearchEnginesDialog *ui;

    SearchEnginesManager* m_manager;
};

#endif // SEARCHENGINESDIALOG_H
