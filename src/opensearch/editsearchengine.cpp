#include "editsearchengine.h"
#include "ui_editsearchengine.h"

EditSearchEngine::EditSearchEngine(const QString &title, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditSearchEngine)
{
    setWindowTitle(title);
    ui->setupUi(this);

    connect(ui->iconFromFile, SIGNAL(clicked()), this, SLOT(chooseIcon()));
}

QString EditSearchEngine::name()
{
    return ui->name->text();
}

void EditSearchEngine::setName(const QString &name)
{
    ui->name->setText(name);
    ui->name->setCursorPosition(0);
}

QString EditSearchEngine::url()
{
    return ui->url->text();
}

void EditSearchEngine::setUrl(const QString &url)
{
    ui->url->setText(url);
    ui->url->setCursorPosition(0);
}

QString EditSearchEngine::shortcut()
{
    return ui->shortcut->text();
}

void EditSearchEngine::setShortcut(const QString &shortcut)
{
    ui->shortcut->setText(shortcut);
    ui->shortcut->setCursorPosition(0);
}

QIcon EditSearchEngine::icon()
{
    return QIcon(*ui->icon->pixmap());
}

void EditSearchEngine::setIcon(const QIcon &icon)
{
    ui->icon->setPixmap(icon.pixmap(16, 16));
}

void EditSearchEngine::hideIconLabels()
{
    ui->iconLabel->hide();
    ui->editIconFrame->hide();

    resize(width(), sizeHint().height());
}

void EditSearchEngine::chooseIcon()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose icon..."));
    if (path.isEmpty())
        return;

    setIcon(QIcon(path));
}
