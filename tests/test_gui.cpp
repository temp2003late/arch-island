#include <QtTest>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include "monitor.h"
#include "charts.h"
class GuiTest : public QObject {
    Q_OBJECT
private slots:
    void continuousSailing() {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl("qrc:/qml/Island.qml"));
        QScopedPointer<QObject> scene(component.create());
        QVERIFY2(scene, qPrintable(component.errorString()));
        scene->setProperty("animate", false);
        scene->setProperty("width", 1500);
        scene->setProperty("height", 830);
        auto *boat = scene->findChild<QQuickItem *>("sailingBoat");
        QVERIFY(boat);
        QPointF previous = boat->position();
        // Several full circuits, including idle, bursts and unavailable metrics.
        for (int i = 0; i < 7200; ++i) {
            if (i % 300 == 0)
                scene->setProperty("traffic", i % 900 == 0 ? -1.0 : i % 600 == 0 ? 0.0 : 1e9);
            QVERIFY(QMetaObject::invokeMethod(scene.data(), "advanceScene", Q_ARG(QVariant, 0.1)));
            const auto position = boat->position();
            QVERIFY(QLineF(previous, position).length() < 1.0);
            QVERIFY(position.x() >= 165 && position.x() <= 495);
            const auto routeY = boat->property("routeCenterY").toDouble();
            QVERIFY(position.y() > routeY - 36 && position.y() < routeY + 36);
            previous = position;
        }
        const auto time = scene->property("sceneTime").toDouble();
        QTest::qWait(120);
        QCOMPARE(scene->property("sceneTime").toDouble(), time);
        QCOMPARE(boat->position(), previous);
        scene->setProperty("animate", true);
        QTest::qWait(120);
        QVERIFY(scene->property("sceneTime").toDouble() > time);
        QVERIFY(QLineF(previous, boat->position()).length() < 2.0);
    }
    void islandInteraction() {
        QTemporaryDir dir;
        Settings settings(dir.filePath("settings.ini"));
        settings.setMode("night");
        Monitor monitor(&settings);
        QQmlApplicationEngine engine;
        QStringList warnings;
        connect(&engine, &QQmlEngine::warnings, this, [&warnings](const QList<QQmlError> &errors) {
            for (const auto &error : errors) warnings << error.toString();
        });
        engine.rootContext()->setContextProperty("preferences", &settings);
        engine.rootContext()->setContextProperty("monitor", &monitor);
        engine.load(QUrl("qrc:/qml/Main.qml"));
        QCOMPARE(engine.rootObjects().size(), 1);
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QTRY_VERIFY_WITH_TIMEOUT(monitor.cpu() >= 0, 4000);
        QVERIFY(monitor.memoryTotal() > 0);
        auto *scene = window->findChild<QQuickItem *>("islandScene");
        QVERIFY(scene); QVERIFY(scene->property("night").toBool());
        QTRY_VERIFY_WITH_TIMEOUT(scene->property("artworkReady").toBool(),5000);
        QVERIFY(!monitor.cpuHistory().isEmpty());
        QCOMPARE(monitor.cpuHistory().last().toDouble(),monitor.cpu());
        QTest::qWait(200);
        const auto screenshotDir = qEnvironmentVariable("ARCH_ISLAND_SCREENSHOTS");
        const int previewWidth = qEnvironmentVariableIntValue("ARCH_ISLAND_PREVIEW_WIDTH");
        const int previewHeight = qEnvironmentVariableIntValue("ARCH_ISLAND_PREVIEW_HEIGHT");
        if (previewWidth >= 800 && previewHeight >= 640) {
            window->resize(previewWidth, previewHeight);
            QTest::qWait(200);
        }
        const int previewWait = qEnvironmentVariableIntValue("ARCH_ISLAND_PREVIEW_WAIT_MS");
        if (previewWait > 0 && previewWait <= 65000) QTest::qWait(previewWait);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/island.png"));
        auto *volcano = window->findChild<QQuickItem *>("volcanoButton");
        QVERIFY(volcano);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            volcano->mapToScene(QPointF(volcano->width()/2,volcano->height()/2)).toPoint());
        QTRY_VERIFY(monitor.processesOpen()); QVERIFY(!monitor.sortByMemory());
        QTRY_VERIFY_WITH_TIMEOUT(monitor.processes()->rowCount() > 0, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(monitor.processes()->data(monitor.processes()->index(0), ProcessModel::CpuRole).toDouble() >= 0, 5000);
        QTest::qWait(400);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/processes.png"));
        auto *drawer = window->findChild<QObject *>("processesDrawer"); QVERIFY(drawer);
        QVERIFY(QMetaObject::invokeMethod(drawer,"close"));
        QTRY_VERIFY(!monitor.processesOpen());
        auto *port = window->findChild<QQuickItem *>("portButton"); QVERIFY(port);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            port->mapToScene(QPointF(port->width()/2,port->height()/2)).toPoint());
        QTRY_VERIFY(monitor.processesOpen()); QVERIFY(monitor.sortByMemory());
        QVERIFY(QMetaObject::invokeMethod(drawer,"close"));
        QTRY_VERIFY(!monitor.processesOpen());
        const auto rowCount = monitor.processes()->rowCount();
        QTest::qWait(2200); QCOMPARE(monitor.processes()->rowCount(), rowCount);
        QVERIFY(QMetaObject::invokeMethod(window,"openSettings"));
        auto *dialog = window->findChild<QObject *>("settingsDialog"); QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        settings.setMode("day"); QTRY_VERIFY(!scene->property("night").toBool());
        settings.setReducedMotion(true); QTRY_VERIFY(!scene->property("animate").toBool());
        QTest::qWait(200);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/settings.png"));
        QVERIFY(QMetaObject::invokeMethod(dialog,"close"));
        QTest::qWait(400);
        settings.setReducedMotion(false);
        window->resize(800,640); QTest::qWait(200);
        QVERIFY(scene->width() > 0 && scene->height() > 0);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/small.png"));
        window->showMinimized();
        QTRY_VERIFY(!scene->property("animate").toBool());
        window->showNormal();
        QTRY_VERIFY(scene->property("animate").toBool());
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }
};
int main(int argc, char **argv) {
    QGuiApplication app(argc,argv);
    QCoreApplication::setOrganizationName("ArchIslandTests");
    QCoreApplication::setApplicationName("gui-tests");
    QQuickStyle::setStyle("Basic");
    registerVisualTypes();
    GuiTest test;
    return QTest::qExec(&test,argc,argv);
}
#include "test_gui.moc"
