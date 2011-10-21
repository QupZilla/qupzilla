#ifndef EDITSEARCHENGINES_H
#define EDITSEARCHENGINES_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
    class EditSearchEngine;
}

class EditSearchEngine : public QDialog
{
    Q_OBJECT
public:
    explicit EditSearchEngine(const QString &title, QWidget* parent = 0);

    void setName(const QString &name);
    void setUrl(const QString &url);
    void setShortcut(const QString &shortcut);
    void setIcon(const QIcon &icon);

    QString name();
    QString url();
    QString shortcut();
    QIcon icon();

    void hideIconLabels();

signals:

public slots:

private slots:
    void chooseIcon();

private:
    Ui::EditSearchEngine* ui;

};

#endif // EDITSEARCHENGINES_H
