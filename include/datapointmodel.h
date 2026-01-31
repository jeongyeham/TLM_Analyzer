#ifndef TLMANALYZER_DATAPOINTMODEL_H
#define TLMANALYZER_DATAPOINTMODEL_H

#include <QAbstractListModel>
#include <QModelIndex>
#include "datapoint.h"
#include <QVector>

class DataPointModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        SpacingRole = Qt::UserRole + 1,
        ResistanceRole,
        CurrentRole,
        EnabledRole
    };

    explicit DataPointModel(QObject *parent = nullptr);

    // Basic model interface
    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // Helpers to manage underlying data
    void setDataPoints(const QVector<DataPoint> &points);
    [[nodiscard]] const QVector<DataPoint>& dataPoints() const { return m_points; }

    // QML-friendly accessors
    Q_INVOKABLE [[nodiscard]] QVariantMap get(int index) const;
    Q_INVOKABLE [[nodiscard]] int count() const { return rowCount(QModelIndex()); }

private:
    QVector<DataPoint> m_points;
};

#endif // TLMANALYZER_DATAPOINTMODEL_H
