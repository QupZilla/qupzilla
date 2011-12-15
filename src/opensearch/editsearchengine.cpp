/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca <nowrep@gmail.com>
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
    if (path.isEmpty()) {
        return;
    }

    setIcon(QIcon(path));
}
