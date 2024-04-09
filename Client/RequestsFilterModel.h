#pragma once

#include <QSortFilterProxyModel>

class RequestsFilterModel : public QSortFilterProxyModel
{
public:
    RequestsFilterModel(QObject *parent = nullptr);
    virtual ~RequestsFilterModel();

public:
    void setFilterStr(const QString& str);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_filterRow;
};
