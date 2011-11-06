/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2011  David Rosca
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
#ifndef PAGESCREEN_H
#define PAGESCREEN_H

#include <QDialog>
#include <QWebFrame>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QFileDialog>

namespace Ui
{
class PageScreen;
}

class WebView;
class PageScreen : public QDialog
{
    Q_OBJECT

public:
    explicit PageScreen(WebView* view, QWidget* parent);
    ~PageScreen();

private slots:
    void buttonClicked(QAbstractButton* b);

private:
    void createPixmap();

    Ui::PageScreen* ui;
    WebView* m_view;
    QPixmap m_pagePixmap;
};

#endif // PAGESCREEN_H
