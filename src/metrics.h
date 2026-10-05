#pragma once
#include <QString>
#include <QMap>
#include <QVector>
#include <optional>
#include <array>
namespace Metrics {
struct Cpu { std::array<quint64, 8> counters{}; };
struct Memory { quint64 total = 0, available = 0; };
struct Network { quint64 received = 0, sent = 0; };
struct NetworkRates { double received = -1, sent = -1; };
class NetworkSampler {
public:
    NetworkRates sample(const QString &interface, std::optional<Network> current, qint64 timeNs);
private:
    QString m_interface;
    std::optional<Network> m_previous;
    qint64 m_timeNs = 0;
};
struct Process {
    qint64 pid = 0;
    QString name;
    quint64 startTime = 0, ticks = 0, residentBytes = 0;
    double cpu = -1;
};
struct ProcessSnapshot {
    QVector<Process> processes;
    qint64 timeNs = 0;
    bool available = false;
};
std::optional<Cpu> parseCpu(const QByteArray &text);
std::optional<double> cpuUsage(const Cpu &before, const Cpu &after);
std::optional<Memory> parseMemory(const QByteArray &text);
QMap<QString, Network> parseNetwork(const QByteArray &text);
std::optional<double> rate(quint64 before, quint64 after, qint64 intervalNs);
std::optional<Process> parseProcess(const QByteArray &text, quint64 pageSize);
void calculateProcessCpu(ProcessSnapshot &current, const ProcessSnapshot &previous, long ticksPerSecond);
ProcessSnapshot scanProcesses();
qint64 monotonicNs();
std::optional<QByteArray> readFile(const QString &path);
}
