/* ============================================================
* QupZilla - Qt web browser
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
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
#ifndef ADBLOCKICON_H
#define ADBLOCKICON_H

#include "qzcommon.h"
#include "abstractbuttoninterface.h"

class QUrl;
class QMenu;
class QAction;

class AdBlockRule;

class QUPZILLA_EXPORT AdBlockIcon : public AbstractButtonInterface
{
    Q_OBJECT

public:
    explicit AdBlockIcon(QObject *parent = nullptr);
    ~AdBlockIcon();

    QString id() const override;
    QString name() const override;

    void popupBlocked(const QString &ruleString, const QUrl &url);
    QAction* menuAction();

public slots:
    void setEnabled(bool enabled);
    void createMenu(QMenu* menu = 0);

private slots:
    void toggleCustomFilter();
    void animateIcon();
    void stopAnimation();

private:
    void clicked(ClickController *controller);

    QAction* m_menuAction;

    QVector<QPair<AdBlockRule*, QUrl> > m_blockedPopups;
    QTimer* m_flashTimer;

    int m_timerTicks;
    bool m_enabled;
};

#endif // ADBLOCKICON_H
