/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013  David Rosca <nowrep@gmail.com>
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
#include "spellcheckdialog.h"
#include "ui_spellcheckdialog.h"
#include "settings.h"
#include "speller.h"
#include "mainapplication.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>

SpellCheckDialog::SpellCheckDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SpellCheckDialog)
    , m_listChanged(false)
{
    ui->setupUi(this);

    ui->dictPath->setText(mApp->speller()->dictionaryPath());

    QFile file(mApp->currentProfilePath() + "userdictionary.txt");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "SpellCheckDialog: Cannot open file" << file.fileName() << "for reading!";
    }
    else {
        QString word;
        QTextStream stream(&file);

        while (!stream.atEnd()) {
            stream >> word;
            word = word.trimmed();

            if (!word.isEmpty()) {
                ui->userDictList->insertItem(0, word);
            }
        }

        file.close();
    }

    connect(ui->changeDictPath, SIGNAL(clicked()), this, SLOT(changeDictionaryPath()));
    connect(ui->add, SIGNAL(clicked()), this, SLOT(addUserWord()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeUserWord()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));

    ui->userDictList->setFocus();
}

void SpellCheckDialog::changeDictionaryPath()
{
    const QString &path = QFileDialog::getExistingDirectory(this, tr("Choose dictionary path..."),
                          ui->dictPath->text());

    if (!path.isEmpty()) {
        ui->dictPath->setText(path);
    }
}

void SpellCheckDialog::addUserWord()
{
    const QString &word = QInputDialog::getText(0, tr("Add new word..."), tr("Add new word:"));

    if (!word.isEmpty()) {
        ui->userDictList->addItem(word);
        ui->userDictList->setCurrentRow(ui->userDictList->count() - 1);

        m_listChanged = true;
    }
}

void SpellCheckDialog::removeUserWord()
{
    QListWidgetItem* item = ui->userDictList->currentItem();

    if (!item) {
        return;
    }

    m_listChanged = true;
    delete item;
}

void SpellCheckDialog::saveSettings()
{
    // Save only when changed

    if (ui->dictPath->text() != mApp->speller()->dictionaryPath()) {
        Settings settings;
        settings.beginGroup("SpellCheck");
        settings.setValue("dictionaryPath", ui->dictPath->text());
        settings.endGroup();
    }

    if (!m_listChanged) {
        return;
    }

    QFile file(mApp->currentProfilePath() + "userdictionary.txt");
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "SpellCheckDialog: Cannot open file" << file.fileName() << "for reading!";
        return;
    }

    QTextStream stream(&file);
    int count = ui->userDictList->count();

    for (int i = 0; i < count; ++i) {
        const QString &word = ui->userDictList->item(i)->text();
        stream << word << endl;
    }

    file.close();
}

SpellCheckDialog::~SpellCheckDialog()
{
    delete ui;
}
