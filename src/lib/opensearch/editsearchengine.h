/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#ifndef EDITSEARCHENGINES_H
#define EDITSEARCHENGINES_H

#include "qzcommon.h"

#include <QDialog>

namespace Ui
{
class EditSearchEngine;
}

class QUPZILLA_EXPORT EditSearchEngine : public QDialog
{
    Q_OBJECT
public:
    explicit EditSearchEngine(const QString &title, QWidget* parent = 0);

    void setName(const QString &name);
    void setUrl(const QString &url);
    void setPostData(const QString &postData);
    void setShortcut(const QString &shortcut);
    void setIcon(const QIcon &icon);

    QString name();
    QString url();
    QString postData();
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
