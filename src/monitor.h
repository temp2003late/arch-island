#pragma once
#include <QObject>
#include <QTimer>
#include <QFutureWatcher>
#include "settings.h"
#include "processmodel.h"
#include "history.h"
class Monitor final : public QObject {
    Q_OBJECT
    Q_PROPERTY(double cpu READ cpu NOTIFY updated)
    Q_PROPERTY(double memoryUsed READ memoryUsed NOTIFY updated)
    Q_PROPERTY(double memoryTotal READ memoryTotal NOTIFY updated)
    Q_PROPERTY(double receiveRate READ receiveRate NOTIFY updated)
    Q_PROPERTY(double sendRate READ sendRate NOTIFY updated)
    Q_PROPERTY(QStringList interfaces READ interfaces NOTIFY updated)
    Q_PROPERTY(QString activeInterface READ activeInterface NOTIFY updated)
    Q_PROPERTY(bool night READ night NOTIFY updated)
    Q_PROPERTY(QString status READ status NOTIFY updated)
    Q_PROPERTY(QString processStatus READ processStatus NOTIFY updated)
    Q_PROPERTY(ProcessModel* processes READ processes CONSTANT)
    Q_PROPERTY(bool processesOpen READ processesOpen WRITE setProcessesOpen NOTIFY updated)
    Q_PROPERTY(bool sortByMemory READ sortByMemory WRITE setSortByMemory NOTIFY updated)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY sampled)
    Q_PROPERTY(QVariantList memoryHistory READ memoryHistory NOTIFY sampled)
    Q_PROPERTY(QVariantList receiveHistory READ receiveHistory NOTIFY sampled)
    Q_PROPERTY(QVariantList sendHistory READ sendHistory NOTIFY sampled)
public:
    explicit Monitor(Settings *settings, QObject *parent = nullptr);
    ~Monitor() override;
    double cpu() const { return m_cpu; }
    double memoryUsed() const { return m_used; }
    double memoryTotal() const { return m_total; }
    double receiveRate() const { return m_rx; }
    double sendRate() const { return m_tx; }
    QStringList interfaces() const { return m_interfaces; }
    QString activeInterface() const { return m_interface; }
    bool night() const { return m_night; }
    QString status() const { return m_status; }
    QString processStatus() const { return m_processStatus; }
    ProcessModel *processes() { return &m_model; }
    bool processesOpen() const { return m_open; }
    bool sortByMemory() const { return m_byMemory; }
    void setProcessesOpen(bool value);
    void setSortByMemory(bool value);
    QVariantList cpuHistory() const { return m_cpuHistory.values(); }
    QVariantList memoryHistory() const { return m_memoryHistory.values(); }
    QVariantList receiveHistory() const { return m_receiveHistory.values(); }
    QVariantList sendHistory() const { return m_sendHistory.values(); }
signals:
    void updated();
    void sampled();
private:
    void sample();
    void updateAppearance();
    void scan();
    Settings *m_settings;
    QTimer m_timer;
    ProcessModel m_model;
    QFutureWatcher<Metrics::ProcessSnapshot> m_worker;
    Metrics::ProcessSnapshot m_previousProcesses;
    std::optional<Metrics::Cpu> m_previousCpu;
    Metrics::NetworkSampler m_networkSampler;
    SampleHistory m_cpuHistory, m_memoryHistory, m_receiveHistory, m_sendHistory;
    QString m_historyInterface;
    QString m_interface, m_status, m_processStatus;
    QStringList m_interfaces;
    double m_cpu = -1, m_used = -1, m_total = -1, m_rx = -1, m_tx = -1;
    bool m_night = false, m_open = false, m_byMemory = false, m_busy = false;
    int m_tick = 0;
    quint64 m_generation = 0, m_scanGeneration = 0;
};
