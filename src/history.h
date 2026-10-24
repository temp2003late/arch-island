#pragma once
#include <QVariantList>
#include <cmath>
#include <optional>

// Exactly one bounded, real sample per monitor tick. Negative values mark gaps.
class SampleHistory {
public:
    static constexpr int Capacity = 60;
    void append(double value) {
        if (m_values.size() == Capacity) m_values.removeFirst();
        m_values.append(std::isfinite(value) && value >= 0 ? value : -1.0);
    }
    // A discontinuity is not a measured zero. Break the plotted line when
    // sampling stalls, rather than connecting data across an unobserved gap.
    void appendAt(double value, qint64 timeNs) {
        if (m_lastTime && (timeNs <= *m_lastTime || timeNs - *m_lastTime > 2000000000LL))
            append(-1);
        append(value);
        m_lastTime = timeNs;
    }
    void clear() { m_values.clear(); m_lastTime.reset(); }
    QVariantList values() const { return m_values; }
private:
    QVariantList m_values;
    std::optional<qint64> m_lastTime;
};
