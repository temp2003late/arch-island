#include "monitor.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>
#include <QFileInfo>
#include <unistd.h>
#include <QStorageInfo>
#include <QDir>
#include <QSysInfo>
#include <cmath>
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
QString Monitor::hostName() const { return QSysInfo::machineHostName(); }
QString Monitor::kernelVersion() const { return QSysInfo::kernelVersion(); }
void Monitor::sample() {
    QStringList errors;
    // Directory names suffice for a visible process count; detailed process
    // sampling remains asynchronous and only runs while the drawer is open.
    const QDir proc("/proc");
    m_processCount = proc.isReadable() ? 0 : -1;
    if (m_processCount >= 0) {
        for (const auto &name : proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort)) {
            bool numeric = false;
            const auto pid = name.toULongLong(&numeric);
            if (numeric && pid > 0) ++m_processCount;
        }
    }
    const auto cpuText = Metrics::readFile("/proc/stat");
    const auto currentCpu = cpuText ? Metrics::parseCpu(*cpuText) : std::nullopt;
    m_cpu = currentCpu && m_previousCpu ? Metrics::cpuUsage(*m_previousCpu, *currentCpu).value_or(-1) : -1;
    m_previousCpu = currentCpu;
    if (m_cpu < 0) errors << (currentCpu ? "CPU waiting for a valid interval" : "CPU unavailable");
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
    else {
        QStringList physical;
        for (const auto &name : m_interfaces)
            if (QFileInfo::exists("/sys/class/net/" + name + "/device")) physical << name;
        m_interface = Metrics::selectNetworkInterface(m_interfaces, m_interface,
            Metrics::readFile("/proc/net/route").value_or(QByteArray()), physical);
    }
    const auto it = nets.constFind(m_interface);
    const auto currentNetwork = it == nets.cend() ? std::nullopt : std::optional<Metrics::Network>(*it);
    const auto rates = m_networkSampler.sample(m_interface, currentNetwork, now);
    m_rx = rates.received; m_tx = rates.sent;
    if (!currentNetwork) errors << "Network unavailable";
    else {
        if (m_rx < 0) errors << "Network receive waiting for a valid interval";
        if (m_tx < 0) errors << "Network send waiting for a valid interval";
    }
    if (m_storageTick == 0) {
        const QStorageInfo disk(QDir::rootPath());
        const bool valid = disk.isValid() && disk.isReady() && disk.bytesTotal() > 0
            && disk.bytesFree() >= 0 && disk.bytesFree() <= disk.bytesTotal();
        m_storageTotal = valid ? double(disk.bytesTotal()) : -1;
        m_storageUsed = valid ? double(disk.bytesTotal() - disk.bytesFree()) : -1;
        m_storageFormat = valid ? QString::fromLatin1(disk.fileSystemType()) : QString();
    }
    m_storageTick = (m_storageTick + 1) % 10;
    if (m_storageTotal < 0) errors << "Root filesystem unavailable";
    m_uptime = -1;
    if (const auto text = Metrics::readFile("/proc/uptime")) {
        bool ok = false;
        const double seconds = text->simplified().split(' ').value(0).toDouble(&ok);
        if (ok && std::isfinite(seconds) && seconds >= 0) m_uptime = seconds;
    }
    m_alerts = errors;
    if (m_storageTotal > 0 && m_storageUsed / m_storageTotal >= .9)
        m_alerts << "Root filesystem is at least 90% full";
    m_status = errors.isEmpty() ? "Live · sampled every second" : errors.join(" · ");
    m_cpuHistory.appendAt(m_cpu, now);
    m_memoryHistory.appendAt(m_total > 0 ? m_used/m_total*100 : -1, now);
    if (m_historyInterface != m_interface) {
        m_receiveHistory.clear(); m_sendHistory.clear(); m_historyInterface = m_interface;
    }
    m_receiveHistory.appendAt(m_rx, now); m_sendHistory.appendAt(m_tx, now);
    emit sampled();
    if (m_open) { m_tick = (m_tick + 1) % 2; if (m_tick == 0) scan(); }
    updateAppearance();
}
void Monitor::updateAppearance() {
    const int hour = QDateTime::currentDateTime().time().hour();
    m_night = m_settings->mode() == "night" || (m_settings->mode() == "auto" && (hour < 7 || hour >= 19));
    emit updated();
}
