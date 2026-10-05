#include "metrics.h"
#include <QFile>
#include <QDir>
#include <chrono>
#include <limits>
#include <unistd.h>
namespace Metrics {
std::optional<QByteArray> readFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    const auto data = file.readAll();
    if (file.error() != QFileDevice::NoError || data.isEmpty()) return std::nullopt;
    return data;
}
qint64 monotonicNs() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
static std::optional<quint64> number(const QByteArray &s) {
    if (s.isEmpty() || s.startsWith('-')) return std::nullopt;
    bool ok = false; const auto v = s.toULongLong(&ok);
    if (!ok) return std::nullopt;
    return v;
}
std::optional<Cpu> parseCpu(const QByteArray &text) {
    for (const auto &line : text.split('\n')) {
        const auto fields = line.simplified().split(' ');
        if (fields.isEmpty() || fields[0] != "cpu") continue;
        if (fields.size() < 5) return std::nullopt;
        Cpu c;
        for (int i = 0; i < 8 && i + 1 < fields.size(); ++i) {
            const auto v = number(fields[i+1]); if (!v) return std::nullopt;
            c.counters[i] = *v;
        }
        return c;
    }
    return std::nullopt;
}
std::optional<double> cpuUsage(const Cpu &a, const Cpu &b) {
    long double total = 0, idle = 0;
    for (int i = 0; i < 8; ++i) {
        if (b.counters[i] < a.counters[i]) return std::nullopt;
        const auto d = b.counters[i] - a.counters[i]; total += d;
        if (i == 3 || i == 4) idle += d;
    }
    if (total <= 0) return std::nullopt;
    return double(100 * (total - idle) / total);
}
std::optional<Memory> parseMemory(const QByteArray &text) {
    std::optional<quint64> total, available;
    for (const auto &line : text.split('\n')) {
        const auto f = line.simplified().split(' ');
        if (f.isEmpty() || (f[0] != "MemTotal:" && f[0] != "MemAvailable:")) continue;
        if (f.size() != 3 || f[2] != "kB") return std::nullopt;
        auto v = number(f[1]);
        if (!v || *v > std::numeric_limits<quint64>::max()/1024) return std::nullopt;
        (f[0] == "MemTotal:" ? total : available) = *v * 1024;
    }
    if (!total || !available || *total == 0 || *available > *total) return std::nullopt;
    return Memory{*total, *available};
}
QMap<QString, Network> parseNetwork(const QByteArray &text) {
    QMap<QString, Network> result;
    for (const auto &line : text.split('\n')) {
        const auto colon = line.indexOf(':'); if (colon < 0) continue;
        const auto name = QString::fromUtf8(line.left(colon).trimmed());
        const auto f = line.mid(colon+1).simplified().split(' ');
        if (name.isEmpty() || f.size() != 16) continue;
        bool valid = true;
        for (const auto &field : f) if (!number(field)) valid = false;
        if (valid) result.insert(name, Network{*number(f[0]), *number(f[8])});
    }
    return result;
}
std::optional<double> rate(quint64 a, quint64 b, qint64 ns) {
    if (ns <= 0 || b < a) return std::nullopt;
    return double(b - a) / (double(ns) / 1e9);
}
NetworkRates NetworkSampler::sample(const QString &interface, std::optional<Network> current, qint64 timeNs) {
    NetworkRates result;
    if (current && m_previous && interface == m_interface) {
        result.received = rate(m_previous->received, current->received, timeNs-m_timeNs).value_or(-1);
        result.sent = rate(m_previous->sent, current->sent, timeNs-m_timeNs).value_or(-1);
    }
    m_interface = interface; m_previous = current; m_timeNs = timeNs;
    return result;
}
std::optional<Process> parseProcess(const QByteArray &text, quint64 pageSize) {
    const auto left = text.indexOf('('), right = text.lastIndexOf(')');
    if (left <= 0 || right <= left || pageSize == 0) return std::nullopt;
    const auto pid = number(text.left(left).trimmed());
    const auto f = text.mid(right+1).simplified().split(' ');
    // Tail begins with field 3 (state); indices 11/12 = utime/stime, 19 = starttime, 21 = rss.
    if (!pid || *pid == 0 || *pid > quint64(std::numeric_limits<qint64>::max()) || f.size() < 22 || f[0].size() != 1) return std::nullopt;
    const auto user = number(f[11]), system = number(f[12]), start = number(f[19]), rss = number(f[21]);
    if (!user || !system || !start || !rss || *rss > std::numeric_limits<quint64>::max()/pageSize || *system > std::numeric_limits<quint64>::max()-*user) return std::nullopt;
    return Process{qint64(*pid), QString::fromUtf8(text.mid(left+1, right-left-1)), *start, *user+*system, *rss*pageSize, -1};
}
void calculateProcessCpu(ProcessSnapshot &now, const ProcessSnapshot &old, long hz) {
    if (!old.available || !now.available || hz <= 0 || now.timeNs <= old.timeNs) return;
    QMap<qint64, Process> map;
    for (const auto &p : old.processes) map.insert(p.pid, p);
    const double seconds = double(now.timeNs - old.timeNs) / 1e9;
    for (auto &p : now.processes) {
        const auto it = map.constFind(p.pid);
        if (it != map.cend() && p.startTime == it->startTime && p.ticks >= it->ticks)
            p.cpu = 100.0 * double(p.ticks-it->ticks) / double(hz) / seconds;
    }
}
ProcessSnapshot scanProcesses() {
    ProcessSnapshot result;
    const QDir proc("/proc");
    const long pageSize = sysconf(_SC_PAGESIZE);
    // /proc/stat distinguishes an inaccessible proc mount from a genuinely empty list.
    if (pageSize <= 0 || !readFile("/proc/stat")) return result;
    result.available = true;
    for (const auto &entry : proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        bool ok; entry.toLongLong(&ok); if (!ok) continue;
        const auto data = readFile("/proc/" + entry + "/stat");
        if (!data) continue; // Exited or denied; no synthetic row.
        const auto p = parseProcess(*data, quint64(pageSize));
        if (p) result.processes.push_back(*p);
    }
    result.timeNs = monotonicNs();
    return result;
}
}
