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
#ifndef LOCATIONCOMPLETERVIEW_H
#define LOCATIONCOMPLETERVIEW_H

#include <QListView>

#include "qzcommon.h"

class LoadRequest;
class LocationCompleterDelegate;

class QTimer;
class QHBoxLayout;

class QUPZILLA_EXPORT LocationCompleterView : public QWidget
{
    Q_OBJECT
public:
    explicit LocationCompleterView();

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    QItemSelectionModel *selectionModel() const;

    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex &index);

    void setOriginalText(const QString &originalText);

    void adjustSize();

    bool eventFilter(QObject* object, QEvent* event);

signals:
    void closed();
    void searchEnginesDialogRequested();
    void loadRequested(const LoadRequest &request);

    void indexActivated(const QModelIndex &index);
    void indexCtrlActivated(const QModelIndex &index);
    void indexShiftActivated(const QModelIndex &index);
    void indexDeleteRequested(const QModelIndex &index);

public slots:
    void close();

private:
    void setupSearchEngines();
    void openSearchEnginesDialog();

    QListView *m_view;
    LocationCompleterDelegate *m_delegate;
    QHBoxLayout *m_searchEnginesLayout;
    QString m_originalText;
    int m_resizeHeight = -1;
    QTimer *m_resizeTimer = nullptr;
    bool m_forceResize = true;
};

#endif // LOCATIONCOMPLETERVIEW_H
