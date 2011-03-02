#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#if defined(QT_NO_DEBUG) & !defined(QT_NO_DEBUG_OUTPUT)
#define QT_NO_DEBUG_OUTPUT
#endif

#include <QDialog>

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private slots:
    void showAbout();
    void showAuthors();
    void buttonClicked();

private:
    Ui::AboutDialog *ui;

    QString m_aboutHtml;
    QString m_authorsHtml;
};

#endif // ABOUTDIALOG_H
