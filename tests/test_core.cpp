#include <QtTest>
#include "metrics.h"
#include "settings.h"
#include "processmodel.h"
#include "history.h"
#include "monitor.h"
class CoreTest : public QObject {
    Q_OBJECT
private slots:
#ifdef ISLAND_HAS_DBUS
    void systemMotionHints() {
        QTemporaryDir dir;
        Settings settings(dir.filePath("motion.ini"));
        const auto hint=[&](QString group,QString key,QVariant value) {
            QVERIFY(QMetaObject::invokeMethod(&settings,"portalSettingChanged",Qt::DirectConnection,
                Q_ARG(QString,group),Q_ARG(QString,key),Q_ARG(QDBusVariant,QDBusVariant(value))));
        };
        hint("org.gnome.desktop.interface","enable-animations",false);
        QVERIFY(settings.systemReducedMotion());
        hint("org.kde.kdeglobals.KDE","AnimationDurationFactor",0.0);
        hint("org.gnome.desktop.interface","enable-animations",true);
        QVERIFY(settings.systemReducedMotion());
        hint("org.kde.kdeglobals.KDE","AnimationDurationFactor",1.0);
        QVERIFY(!settings.systemReducedMotion());
        hint("org.gnome.desktop.interface","enable-animations","invalid");
        QVERIFY(!settings.systemReducedMotion());
        QVERIFY(!settings.reducedMotion()); // Does not overwrite the user's choice.
    }
#endif
    void boundedHistory() {
        SampleHistory history;
        QVERIFY(history.values().isEmpty());
        for (int i=0;i<100;++i) history.append(i);
        QCOMPARE(history.values().size(),SampleHistory::Capacity);
        QCOMPARE(history.values().first().toDouble(),40.0);
        QCOMPARE(history.values().last().toDouble(),99.0);
        history.append(-1); QCOMPARE(history.values().last().toDouble(),-1.0);
        history.clear(); QVERIFY(history.values().isEmpty());
    }
    void historyDiscontinuity() {
        SampleHistory history;
        history.appendAt(10, 1000000000LL);
        history.appendAt(20, 2000000000LL);
        history.appendAt(30, 5500000000LL);
        QCOMPARE(history.values(), QVariantList({10.0, 20.0, -1.0, 30.0}));
        history.appendAt(40, 5500000000LL);
        QCOMPARE(history.values()[4].toDouble(), -1.0);
        history.clear();
        history.appendAt(50, 9000000000LL);
        QCOMPARE(history.values(), QVariantList({50.0}));
        for (int i=0; i<100; ++i) history.appendAt(i, 12000000000LL+i*3000000000LL);
        QCOMPARE(history.values().size(), SampleHistory::Capacity);
        QCOMPARE(history.values().last().toDouble(), 99.0);
    }
    void defaultRouteSelection() {
        const QStringList devices{"eth0", "wlan0", "tun0", "lo"};
        const QByteArray routes = "eth0 00000000 0100000A 0003 0 0 600 00000000 0 0 0\n"
                                  "wlan0 00000000 0100000A 0003 0 0 100 00000000 0 0 0\n";
        QCOMPARE(Metrics::selectNetworkInterface(devices, "eth0", routes, {"eth0"}), QString("wlan0"));
        const auto vpn = routes + "tun0 00000000 00000000 0001 0 0 50 00000000 0 0 0\n";
        QCOMPARE(Metrics::selectNetworkInterface(devices, "wlan0", vpn, {"eth0"}), QString("tun0"));
        QCOMPARE(Metrics::selectNetworkInterface(devices, "tun0", routes, {"eth0"}), QString("wlan0"));
        const QByteArray ties = "eth0 00000000 00000000 0001 0 0 100 00000000\n"
                                "wlan0 00000000 00000000 0001 0 0 100 00000000\n";
        QCOMPARE(Metrics::selectNetworkInterface(devices, "wlan0", ties, {}), QString("wlan0"));
        const QByteArray invalid = "tun0 00000000 00000000 0201 0 0 0 00000000\n"
                                   "eth0 00000000 00000000 0000 0 0 0 00000000\n"
                                   "lo 00000000 00000000 0001 0 0 0 00000000\n"
                                   "gone 00000000 00000000 0001 0 0 0 00000000\n"
                                   "tun0 00000000 00000000 0001 0 0 -1 00000000\n"
                                   "tun0 00000000 00000000 0001 0 0 0 000000FF\n";
        QCOMPARE(Metrics::selectNetworkInterface(devices, "wlan0", invalid, {"eth0"}), QString("wlan0"));
        QCOMPARE(Metrics::selectNetworkInterface(devices, "gone", invalid, {"eth0"}), QString("eth0"));
        QVERIFY(Metrics::selectNetworkInterface({"lo"}, "lo", {}, {}).isEmpty());
    }
    void cpu() {
        const auto a = Metrics::parseCpu("cpu 10 0 10 80 0 0 0 0 999 999\ncpu0 1 2 3 4\n");
        const auto b = Metrics::parseCpu("cpu 20 0 20 160 0 0 0 0 9999 9999\n");
        QVERIFY(a && b); QCOMPARE(*Metrics::cpuUsage(*a,*b), 20.0);
        QVERIFY(!Metrics::cpuUsage(*b,*a)); QVERIFY(!Metrics::cpuUsage(*a,*a));
        QVERIFY(!Metrics::parseCpu("cpu 1 bad 3 4"));
        QVERIFY(!Metrics::parseCpu("cpu0 1 2 3 4"));
    }
    void memory() {
        const auto m = Metrics::parseMemory("MemFree: 10 kB\nMemAvailable: 25 kB\nMemTotal: 100 kB\n");
        QVERIFY(m); QCOMPARE(m->total-m->available, quint64(75*1024));
        QVERIFY(!Metrics::parseMemory("MemTotal: 100 kB\nMemFree: 10 kB\n"));
        QVERIFY(!Metrics::parseMemory("MemTotal: 10 kB\nMemAvailable: 20 kB\n"));
        QVERIFY(!Metrics::parseMemory("MemTotal: 10 MB\nMemAvailable: 2 kB\n"));
    }
    void network() {
        const auto map = Metrics::parseNetwork("Inter-| Receive\n enp1s0: 100 0 0 0 0 0 0 0 200 0 0 0 0 0 0 0\n lo: 2 0 0 0 0 0 0 0 3 0 0 0 0 0 0 0\n broken: 1 2\n");
        QCOMPARE(map.size(), 2); QCOMPARE(map["enp1s0"].sent, quint64(200));
        QCOMPARE(*Metrics::rate(100, 300, 2000000000), 100.0);
        QVERIFY(!Metrics::rate(100, 300, 0)); QVERIFY(!Metrics::rate(300, 100, 1000000000));
        QVERIFY(!Metrics::rate(0, 1, -1)); QCOMPARE(*Metrics::rate(20,20,1000000000),0.0);
    }
    void process() {
        const auto p = Metrics::parseProcess("42 (odd ) name (worker)) R 1 2 3 4 5 6 7 8 9 10 120 30 0 0 20 0 1 0 999 50000 12",4096);
        QVERIFY(p); QCOMPARE(p->name, QString("odd ) name (worker)"));
        QCOMPARE(p->ticks,quint64(150)); QCOMPARE(p->startTime,quint64(999)); QCOMPARE(p->residentBytes,quint64(49152));
        QVERIFY(!Metrics::parseProcess("42 (gone) R 1 2",4096));
        auto old = Metrics::ProcessSnapshot{{*p},1000000000,true};
        auto now = old; now.timeNs += 2000000000; now.processes[0].ticks += 200;
        Metrics::calculateProcessCpu(now,old,100); QCOMPARE(now.processes[0].cpu,100.0);
        now.processes[0].ticks += 200;
        Metrics::calculateProcessCpu(now,old,100); QCOMPARE(now.processes[0].cpu,200.0);
        now.processes[0].startTime++;
        Metrics::calculateProcessCpu(now,old,100); QCOMPARE(now.processes[0].cpu,-1.0);
        now.processes[0].startTime--; now.processes[0].ticks = 1;
        Metrics::calculateProcessCpu(now,old,100); QCOMPARE(now.processes[0].cpu,-1.0);
        now.processes[0].cpu = 80;
        Metrics::calculateProcessCpu(now,old,0); QCOMPARE(now.processes[0].cpu,-1.0);
        now.processes[0].cpu = 80;
        now.timeNs = old.timeNs;
        Metrics::calculateProcessCpu(now,old,100); QCOMPARE(now.processes[0].cpu,-1.0);
        now.processes.clear(); Metrics::calculateProcessCpu(now,old,100); QVERIFY(now.processes.isEmpty());
    }
    void processModel() {
        ProcessModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QVector<Metrics::Process> rows{{1,"first",10,10,1024,60},{2,"second",20,10,4096,10}};
        model.replace(rows,false);
        QCOMPARE(model.data(model.index(0),ProcessModel::PidRole).toLongLong(),qint64(1));
        model.replace(rows,true);
        QCOMPARE(model.data(model.index(0),ProcessModel::PidRole).toLongLong(),qint64(2));
        QCOMPARE(model.roleNames().value(ProcessModel::NameRole),QByteArray("processName"));
        QVERIFY(!model.data(QModelIndex(),ProcessModel::CpuRole).isValid());
        for (int i=3;i<250;++i) rows.push_back({i,"extra",0,0,0,0});
        model.replace(rows,false); QCOMPARE(model.rowCount(),200);
        model.replace({},false); QCOMPARE(model.rowCount(),0);
    }
    void networkLifecycle() {
        Metrics::NetworkSampler sampler;
        QCOMPARE(sampler.sample("eth0",Metrics::Network{100,200},1000000000).received,-1.0);
        const auto rates = sampler.sample("eth0",Metrics::Network{300,300},3000000000);
        QCOMPARE(rates.received,100.0); QCOMPARE(rates.sent,50.0);
        QCOMPARE(sampler.sample("wlan0",Metrics::Network{10000,20000},4000000000).received,-1.0);
        QCOMPARE(sampler.sample("wlan0",std::nullopt,5000000000).received,-1.0);
        QCOMPARE(sampler.sample("wlan0",Metrics::Network{15000,25000},6000000000).received,-1.0);
        QCOMPARE(sampler.sample("wlan0",Metrics::Network{1,1},7000000000).received,-1.0);
        QCOMPARE(sampler.sample("wlan0",Metrics::Network{3,4},8000000000).received,2.0);
        QCOMPARE(sampler.sample("wlan0",Metrics::Network{4,5},8000000000).received,-1.0);
    }
    void networkDirectionsResetIndependently() {
        Metrics::NetworkSampler sampler;
        sampler.sample("eth0", Metrics::Network{100,200}, 1000000000LL);
        auto sample = sampler.sample("eth0", Metrics::Network{50,300}, 2000000000LL);
        QCOMPARE(sample.received, -1.0);
        QCOMPARE(sample.sent, 100.0);
        sample = sampler.sample("eth0", Metrics::Network{100,10}, 3000000000LL);
        QCOMPARE(sample.received, 50.0);
        QCOMPARE(sample.sent, -1.0);
        sample = sampler.sample("eth0", Metrics::Network{200,210}, 5500000000LL);
        QCOMPARE(sample.received, 40.0);
        QCOMPARE(sample.sent, 80.0);
    }
    void monitoringAvailability() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Settings settings(dir.filePath("monitor.ini"));
        settings.setNetworkInterface("arch-island-nonexistent-test-interface");
        Monitor monitor(&settings);
        QCOMPARE(monitor.cpu(), -1.0);
        QVERIFY(monitor.alerts().contains("CPU waiting for a valid interval"));
        QCOMPARE(monitor.receiveRate(), -1.0);
        QVERIFY(monitor.alerts().contains("Network unavailable"));
        QTRY_VERIFY_WITH_TIMEOUT(monitor.cpu() >= 0, 4000);
        QVERIFY(!monitor.alerts().contains("CPU waiting for a valid interval"));
        QCOMPARE(monitor.activeInterface(), settings.networkInterface());
        QCOMPARE(monitor.receiveHistory().last().toDouble(), -1.0);
        // Loopback exists even on an offline host. Exercise the sampler's first
        // interval after a switch without depending on external connectivity.
        settings.setNetworkInterface("lo");
        QTRY_COMPARE_WITH_TIMEOUT(monitor.activeInterface(), QString("lo"), 4000);
        QCOMPARE(monitor.receiveRate(), -1.0);
        QCOMPARE(monitor.receiveHistory().size(), 1);
        QVERIFY(monitor.alerts().contains("Network receive waiting for a valid interval"));
        QTRY_VERIFY_WITH_TIMEOUT(monitor.receiveRate() >= 0, 4000);
        QVERIFY(!monitor.alerts().contains("Network receive waiting for a valid interval"));
        QVERIFY(!monitor.alerts().contains("Network send waiting for a valid interval"));
    }
    void stalledMonitoringBreaksHistory() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Settings settings(dir.filePath("stall.ini"));
        Monitor monitor(&settings);
        QSignalSpy sampled(&monitor, &Monitor::sampled);
        QTRY_VERIFY_WITH_TIMEOUT(sampled.size() >= 1, 4000);
        const auto before = monitor.cpuHistory().size();
        QTest::qSleep(2300); // Deliberately block the GUI/event thread.
        QCoreApplication::processEvents();
        const auto history = monitor.cpuHistory();
        QCOMPARE(history.size(), before + 2);
        QCOMPARE(history[history.size()-2].toDouble(), -1.0);
        QVERIFY(history.last().toDouble() >= 0);
        QCOMPARE(monitor.memoryHistory()[history.size()-2].toDouble(), -1.0);
    }
    void readErrors() {
        QTemporaryDir dir; QVERIFY(dir.isValid());
        QVERIFY(!Metrics::readFile(dir.filePath("gone")));
        QFile file(dir.filePath("denied")); QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("data"); file.close();
        QVERIFY(file.setPermissions(QFileDevice::WriteOwner));
        QVERIFY(!Metrics::readFile(file.fileName()));
        QVERIFY(file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    }
    void settings() {
        QTemporaryDir dir; QVERIFY(dir.isValid()); const auto path=dir.filePath("test.ini");
        { Settings s(path); s.setMode("night"); s.setNetworkInterface("eth-test"); s.setReducedMotion(true); s.setMode("invalid"); }
        { Settings s(path); QCOMPARE(s.mode(),QString("night")); QCOMPARE(s.networkInterface(),QString("eth-test")); QVERIFY(s.reducedMotion()); }
    }
};
QTEST_GUILESS_MAIN(CoreTest)
#include "test_core.moc"
