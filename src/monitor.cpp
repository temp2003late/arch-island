#include "monitor.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>
#include <QFileInfo>
#include <unistd.h>
Monitor::Monitor(Settings *settings, QObject *p) : QObject(p), m_settings(settings) {
    connect(&m_timer, &QTimer::timeout, this, &Monitor::sample);
    connect(settings, &Settings::changed, this, &Monitor::updateAppearance);
    connect(&m_worker, &QFutureWatcher<Metrics::ProcessSnapshot>::finished, this, [this] {
        m_busy = false;
        if (!m_open || m_scanGeneration != m_generation) { if (m_open) scan(); return; }
        auto result = m_worker.result();
        Metrics::calculateProcessCpu(result, m_previousProcesses, sysconf(_SC_CLK_TCK));
        m_previousProcesses = result;
        m_model.replace(result.processes, m_byMemory);
        m_processStatus = result.available ? QString("%1 readable processes · top 200 · refreshed every 2 s").arg(result.processes.size()) : "Process data unavailable";
        emit updated();
    });
    m_timer.setInterval(1000); m_timer.start(); sample();
}
Monitor::~Monitor() { m_worker.waitForFinished(); }
void Monitor::setProcessesOpen(bool value) {
    if (m_open == value) return;
    m_open = value; ++m_generation; m_previousProcesses = {};
    m_model.replace({}, m_byMemory);
    m_processStatus = "Reading processes… CPU needs two samples";
    if (value) scan();
    emit updated();
}
void Monitor::setSortByMemory(bool value) {
    if (m_byMemory == value) return;
    m_byMemory = value; m_model.replace(m_previousProcesses.processes, value); emit updated();
}
void Monitor::scan() {
    if (!m_open || m_busy) return;
    m_busy = true; m_scanGeneration = m_generation;
    m_worker.setFuture(QtConcurrent::run(&Metrics::scanProcesses));
}
void Monitor::sample() {
    QStringList errors;
    const auto cpuText = Metrics::readFile("/proc/stat");
    const auto currentCpu = cpuText ? Metrics::parseCpu(*cpuText) : std::nullopt;
    m_cpu = currentCpu && m_previousCpu ? Metrics::cpuUsage(*m_previousCpu, *currentCpu).value_or(-1) : -1;
    m_previousCpu = currentCpu;
    if (!currentCpu) errors << "CPU unavailable";
    const auto memText = Metrics::readFile("/proc/meminfo");
    const auto memory = memText ? Metrics::parseMemory(*memText) : std::nullopt;
    m_total = memory ? double(memory->total) : -1;
    m_used = memory ? double(memory->total-memory->available) : -1;
    if (!memory) errors << "RAM unavailable";
    const auto netText = Metrics::readFile("/proc/net/dev");
    const auto nets = netText ? Metrics::parseNetwork(*netText) : QMap<QString, Metrics::Network>{};
    const auto now = Metrics::monotonicNs();
    m_interfaces = nets.keys(); m_interfaces.removeAll("lo");
    const auto choice = m_settings->networkInterface();
    if (!choice.isEmpty()) m_interface = choice;
    else if (!m_interfaces.contains(m_interface)) {
        m_interface.clear();
        // Prefer the default IPv4 route, then a physical device, then exactly one virtual interface.
        const auto routes = Metrics::readFile("/proc/net/route");
        if (routes) for (const auto &line : routes->split('\n')) {
            const auto f = line.simplified().split(' ');
            if (f.size() > 3 && f[1] == "00000000" && m_interfaces.contains(QString::fromUtf8(f[0]))) {
                bool ok = false; const auto flags = f[3].toUInt(&ok, 16);
                if (ok && (flags & 1)) { m_interface = QString::fromUtf8(f[0]); break; }
            }
        }
        if (m_interface.isEmpty()) for (const auto &name : m_interfaces)
            if (QFileInfo::exists("/sys/class/net/" + name + "/device")) { m_interface = name; break; }
        if (m_interface.isEmpty() && !m_interfaces.isEmpty()) m_interface = m_interfaces.first();
    }
    const auto it = nets.constFind(m_interface);
    const auto currentNetwork = it == nets.cend() ? std::nullopt : std::optional<Metrics::Network>(*it);
    const auto rates = m_networkSampler.sample(m_interface, currentNetwork, now);
    m_rx = rates.received; m_tx = rates.sent;
    if (!currentNetwork) errors << "Network unavailable";
    m_status = errors.isEmpty() ? "Live · sampled every second" : errors.join(" · ");
    m_cpuHistory.append(m_cpu);
    m_memoryHistory.append(m_total > 0 ? m_used/m_total*100 : -1);
    if (m_historyInterface != m_interface) {
        m_receiveHistory.clear(); m_sendHistory.clear(); m_historyInterface = m_interface;
    }
    m_receiveHistory.append(m_rx); m_sendHistory.append(m_tx);
    emit sampled();
    if (m_open && ++m_tick % 2 == 0) scan();
    updateAppearance();
}
void Monitor::updateAppearance() {
    const int hour = QDateTime::currentDateTime().time().hour();
    m_night = m_settings->mode() == "night" || (m_settings->mode() == "auto" && (hour < 7 || hour >= 19));
    emit updated();
}
