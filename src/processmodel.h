#pragma once
#include <QAbstractListModel>
#include "metrics.h"
class ProcessModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role { PidRole = Qt::UserRole+1, NameRole, CpuRole, MemoryRole };
    explicit ProcessModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void replace(QVector<Metrics::Process> rows, bool byMemory);
private:
    QVector<Metrics::Process> m_rows;
};
