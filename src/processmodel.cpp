#include "processmodel.h"
#include <algorithm>
int ProcessModel::rowCount(const QModelIndex &p) const { return p.isValid() ? 0 : m_rows.size(); }
QVariant ProcessModel::data(const QModelIndex &i, int role) const {
    if (!i.isValid() || i.row() < 0 || i.row() >= m_rows.size()) return {};
    const auto &p = m_rows[i.row()];
    switch (role) {
    case PidRole: return p.pid;
    case NameRole: return p.name;
    case CpuRole: return p.cpu;
    case MemoryRole: return double(p.residentBytes);
    default: return {};
    }
}
QHash<int, QByteArray> ProcessModel::roleNames() const {
    return {{PidRole,"processPid"},{NameRole,"processName"},{CpuRole,"processCpu"},{MemoryRole,"processMemory"}};
}
void ProcessModel::replace(QVector<Metrics::Process> rows, bool memory) {
    std::sort(rows.begin(), rows.end(), [memory](const auto &a, const auto &b) {
        const auto av = memory ? double(a.residentBytes) : a.cpu;
        const auto bv = memory ? double(b.residentBytes) : b.cpu;
        return av == bv ? a.pid < b.pid : av > bv;
    });
    if (rows.size() > 200) rows.resize(200);
    beginResetModel(); m_rows = std::move(rows); endResetModel();
}
