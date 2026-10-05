#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>
#include "monitor.h"
#include "charts.h"
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ArchIsland");
    QCoreApplication::setApplicationName("arch-island");
    QCoreApplication::setApplicationVersion("0.1.0");
    QQuickStyle::setStyle("Basic");
    registerVisualTypes();
    Settings settings;
    Monitor monitor(&settings);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("preferences", &settings);
    engine.rootContext()->setContextProperty("monitor", &monitor);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [&app](QObject *object, const QUrl &) { if (!object) app.exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl("qrc:/qml/Main.qml"));
    if (engine.rootObjects().isEmpty()) return 1;
    // Bounded smoke run for installed-resource and display-backend verification.
    const int flag = app.arguments().indexOf("--smoke-test");
    if (flag >= 0) {
        bool ok = false;
        const int requested = app.arguments().value(flag+1).toInt(&ok);
        const int seconds = ok && requested > 0 && requested <= 120 ? requested : 4;
        QTimer::singleShot(seconds*1000, &app, &QCoreApplication::quit);
    }
    return app.exec();
}
