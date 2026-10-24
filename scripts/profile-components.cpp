// Diagnostic executable; links the application's resource objects and libraries.
// Each invocation measures one layer freeze in an otherwise live night scene.
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <algorithm>
#include <ctime>
#include "monitor.h"
#include "charts.h"
static double cpuSeconds() {
    timespec time{};
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
    return time.tv_sec + time.tv_nsec/1e9;
}
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    const auto mode = app.arguments().value(1, "baseline");
    const QStringList modes{"baseline", "worldlife", "sailboat", "surface", "signal-lights", "sampling"};
    if (!modes.contains(mode)) { qCritical() << "Unknown mode"; return 2; }
    QTemporaryDir temp;
    if (!temp.isValid()) return 2;
    Settings settings(temp.filePath("settings.ini"));
    settings.setMode("night");
    if (mode == "sampling") {
        QList<double> durations;
        for (int i=0; i<200; ++i) {
            QElapsedTimer timer; timer.start();
            { Monitor monitor(&settings); }
            durations.append(timer.nsecsElapsed()/1e6);
        }
        std::sort(durations.begin(), durations.end());
        QTextStream(stdout) << "Full initial sample including root storage, 200 runs: median="
            << durations[100] << "ms p95=" << durations[189] << "ms max=" << durations.last() << "ms" << Qt::endl;
        return 0;
    }
    QQuickStyle::setStyle("Basic");
    registerVisualTypes();
    Monitor monitor(&settings);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("preferences", &settings);
    engine.rootContext()->setContextProperty("monitor", &monitor);
    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty()) return 2;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (!window) return 2;
    if (mode != "baseline") {
        const QString name = mode == "worldlife" ? "worldLife" : mode == "sailboat" ? "sailboatArtwork"
                           : mode == "surface" ? "livingSurface" : "signalLights";
        auto *item = window->findChild<QObject *>(name);
        if (!item) return 2;
        QQmlExpression freeze(qmlContext(item), item, mode == "signal-lights" ? "model = 0" : "sceneTime = 143.7");
        freeze.evaluate();
        if (freeze.hasError()) { qCritical() << freeze.error(); return 2; }
    }
    QElapsedTimer interval;
    double startCpu = 0;
    QTimer::singleShot(5000, &app, [&] { interval.start(); startCpu = cpuSeconds(); });
    QTimer::singleShot(20000, &app, [&] {
        const double elapsed = interval.nsecsElapsed()/1e9;
        QTextStream(stdout) << mode << ": " << window->width() << "x" << window->height()
            << " elapsed=" << elapsed << "s CPU=" << 100*(cpuSeconds()-startCpu)/elapsed << "% of one core" << Qt::endl;
        app.quit();
    });
    return app.exec();
}
