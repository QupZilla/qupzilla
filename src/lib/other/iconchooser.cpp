/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
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
#include "iconchooser.h"
#include "ui_iconchooser.h"
#include "mainapplication.h"
#include "proxystyle.h"
#include "qztools.h"
#include "sqldatabase.h"

#include <QFileDialog>

IconChooser::IconChooser(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::IconChooser)
{
    ui->setupUi(this);

    ui->iconList->setItemDelegate(new IconChooserDelegate(ui->iconList));

    connect(ui->chooseFile, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(ui->siteUrl, SIGNAL(textChanged(QString)), this, SLOT(searchIcon(QString)));
}

void IconChooser::chooseFile()
{
    const QString fileTypes = QString("%3(*.png *.jpg *.jpeg *.gif)").arg(tr("Image files"));
    const QString path = QzTools::getOpenFileName("IconChooser-ChangeIcon", this, tr("Choose icon..."), QDir::homePath(), fileTypes);

    if (path.isEmpty()) {
        return;
    }

    ui->iconList->clear();
    QIcon icon(path);

    if (!icon.isNull()) {
        QListWidgetItem* item = new QListWidgetItem(ui->iconList);
        item->setIcon(icon);

        ui->iconList->setCurrentItem(item);
    }
}

void IconChooser::searchIcon(const QString &string)
{
    if (string.size() < 4) {
        return;
    }

    ui->iconList->clear();

    QSqlQuery query(SqlDatabase::instance()->database());
    query.prepare(QSL("SELECT icon FROM icons WHERE url GLOB ? LIMIT 20"));
    query.addBindValue(QString(QL1S("*%1*")).arg(QzTools::escapeSqlGlobString(string)));
    query.exec();

    while (query.next()) {
        QImage image = QImage::fromData(query.value(0).toByteArray());
        if (!image.isNull()) {
            QListWidgetItem* item = new QListWidgetItem(ui->iconList);
            item->setIcon(QPixmap::fromImage(image));
        }
    }
}

QIcon IconChooser::getIcon()
{
    QIcon icon;
    int status = QDialog::exec();

    if (status == QDialog::Accepted) {
        QList<QListWidgetItem*> selectedItems = ui->iconList->selectedItems();
        if (!selectedItems.isEmpty()) {
            icon = selectedItems.at(0)->icon();
        }
    }

    // Ensure we are returning 16×16px icon
    if (!icon.isNull()) {
        icon = icon.pixmap(16);
    }

    return icon;
}

IconChooser::~IconChooser()
{
    delete ui;
}

IconChooserDelegate::IconChooserDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void IconChooserDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget* w = opt.widget;
    const QStyle* style = w ? w->style() : QApplication::style();

    // Draw background
    opt.showDecorationSelected = true;
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, w);

    // Draw icon
    const int padding = opt.rect.width() / 4;
    const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    icon.paint(painter, opt.rect.adjusted(padding, padding, -padding, -padding));
}

QSize IconChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QSize(48, 48);
}
