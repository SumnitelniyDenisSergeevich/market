#include "RequestsFilterModel.h"

#include <QDebug>

RequestsFilterModel::RequestsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

RequestsFilterModel::~RequestsFilterModel()
{}

bool RequestsFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex user_id = sourceModel()->index(source_row, 2, source_parent);

    if(!m_filterRow.isEmpty() && sourceModel()->data(user_id).toString() != m_filterRow)
        return false;
    return true;
}

void RequestsFilterModel::setFilterStr(const QString& str)
{
    m_filterRow = str;
}
