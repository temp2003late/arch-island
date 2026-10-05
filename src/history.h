#pragma once
#include <QVariantList>
#include <cmath>

// Exactly one bounded, real sample per monitor tick. Negative values mark gaps.
class SampleHistory {
public:
    static constexpr int Capacity = 60;
    void append(double value) {
        if (m_values.size() == Capacity) m_values.removeFirst();
        m_values.append(std::isfinite(value) && value >= 0 ? value : -1.0);
    }
    void clear() { m_values.clear(); }
    QVariantList values() const { return m_values; }
private:
    QVariantList m_values;
};
