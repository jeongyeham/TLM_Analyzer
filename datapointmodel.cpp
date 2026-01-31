#include "include/datapointmodel.h"
#include <QVariantMap>

DataPointModel::DataPointModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int DataPointModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_points.size();
}

QVariant DataPointModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_points.size()) return {};
    const DataPoint &p = m_points.at(index.row());
    switch (role) {
        case SpacingRole: return p.spacing;
        case ResistanceRole: return p.resistance;
        case CurrentRole: return p.current;
        case EnabledRole: return p.enabled;
        default: return {};
    }
}

QHash<int, QByteArray> DataPointModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SpacingRole] = "spacing";
    roles[ResistanceRole] = "resistance";
    roles[CurrentRole] = "current";
    roles[EnabledRole] = "enabled";
    return roles;
}

void DataPointModel::setDataPoints(const QVector<DataPoint> &points)
{
    beginResetModel();
    m_points = points;
    endResetModel();
}

QVariantMap DataPointModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_points.size()) return map;
    const DataPoint &p = m_points.at(index);
    map["spacing"] = p.spacing;
    map["resistance"] = p.resistance;
    map["current"] = p.current;
    map["enabled"] = p.enabled;
    return map;
}
