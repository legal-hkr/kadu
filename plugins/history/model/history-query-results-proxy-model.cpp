/*
 * %kadu copyright begin%
 * Copyright 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "history-query-results-proxy-model.h"
#include "history-query-results-proxy-model.moc"
#include "model/filter-change.h"

HistoryQueryResultsProxyModel::HistoryQueryResultsProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent), TalkableVisible(true), TitleVisible(true)
{
}

HistoryQueryResultsProxyModel::~HistoryQueryResultsProxyModel()
{
}

bool HistoryQueryResultsProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    if (!TalkableVisible && 0 == sourceColumn)
        return false;
    if (!TitleVisible && 3 == sourceColumn)
        return false;

    return true;
}

void HistoryQueryResultsProxyModel::setTalkableVisible(const bool talkableVisible)
{
    if (TalkableVisible == talkableVisible)
        return;

    KADU_BEGIN_FILTER_CHANGE();
    TalkableVisible = talkableVisible;
    KADU_END_FILTER_CHANGE_BOTH();
}

void HistoryQueryResultsProxyModel::setTitleVisible(const bool titleVisible)
{
    if (TitleVisible == titleVisible)
        return;

    KADU_BEGIN_FILTER_CHANGE();
    TitleVisible = titleVisible;
    KADU_END_FILTER_CHANGE_BOTH();
}
