/* ============================================================
* ImageFinder plugin for QupZilla
* Copyright (C) 2016 Vladislav Tronko <innermous@gmail.com>
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
#include "imagefindersettings.h"
#include "ui_imagefindersettings.h"
#include "imagefinder.h"

ImageFinderSettings::ImageFinderSettings(ImageFinder* finder, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ImageFinderSettings)
    , m_finder(finder)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ImageFinderSettings::accepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ImageFinderSettings::close);
    ui->cboxEngines->setCurrentIndex(m_finder->searchEngine());
}

ImageFinderSettings::~ImageFinderSettings()
{
    delete ui;
}

void ImageFinderSettings::accepted()
{
    int index = ui->cboxEngines->currentIndex();
    ImageFinder::SearchEngine engine = static_cast<ImageFinder::SearchEngine>(index);
    m_finder->setSearchEngine(engine);

    close();
}
