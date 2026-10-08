#include <QtTest>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include <QPainter>
#include <limits>
#include "sailboat.h"
#include "monitor.h"
#include "charts.h"
#include "worldlife.h"
class GuiTest : public QObject {
    Q_OBJECT
private slots:
    void worldLifeAnimation() {
        QTest::failOnWarning(QRegularExpression("^(QColor|QPainter)::"));
        WorldLife life;
        life.setSize(QSizeF(1600,1000));
        auto frame = [&life] {
            QImage image(1600,1000,QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            life.paint(&painter);
            return image;
        };
        const auto day = frame();
        life.setSceneTime(3.0);
        const auto later = frame();
        QVERIFY(day != later);
        QVERIFY(day.copy(400,380,800,330) != later.copy(400,380,800,330));
        QCOMPARE(frame(), later);
        life.setDarkness(1);
        QVERIFY(frame() != later);
        life.setSceneTime(std::numeric_limits<double>::quiet_NaN());
        QCOMPARE(life.sceneTime(),3.0);
    }
    void boatVisibleThroughTurn() {
        QTest::failOnWarning(QRegularExpression("^QColor::"));
        Sailboat boat;
        boat.setSize(QSizeF(190,190));
        boat.setDarkness(1);
        QImage sheet(760,380,QImage::Format_ARGB32_Premultiplied);
        sheet.fill(QColor("#0c2840"));
        for (int degrees=0; degrees<360; degrees+=5) {
            boat.setHeading(degrees);
            QImage frame(190,190,QImage::Format_ARGB32_Premultiplied);
            frame.fill(Qt::transparent);
            { QPainter painter(&frame); boat.paint(&painter); }
            int solid=0, left=190, right=0, top=190, bottom=0;
            for (int y=0;y<190;++y) for (int x=0;x<190;++x) {
                if(qAlpha(frame.pixel(x,y))<128) continue;
                ++solid;
                left=std::min(left,x); right=std::max(right,x);
                top=std::min(top,y); bottom=std::max(bottom,y);
            }
            QVERIFY2(solid>450, qPrintable(QString("Boat vanished at %1 degrees: %2 pixels").arg(degrees).arg(solid)));
            QVERIFY(right-left>24);
            QVERIFY(bottom-top>90);
            QVERIFY(left>0 && right<189 && top>0 && bottom<189);
            if (degrees%45==0) {
                int tile=degrees/45;
                QPainter painter(&sheet);
                painter.drawImage((tile%4)*190,(tile/4)*190,frame);
                painter.setPen(Qt::white);
                painter.drawText((tile%4)*190+8,(tile/4)*190+16,QString::number(degrees)+QChar(0x00b0));
            }
        }
        const auto output=qEnvironmentVariable("ARCH_ISLAND_SCREENSHOTS");
        if(!output.isEmpty()) QVERIFY(sheet.save(output+"/boat-turns.png"));
    }
    void framing() {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl("qrc:/qml/Island.qml"));
        QScopedPointer<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));
        auto *scene = qobject_cast<QQuickItem *>(object.data());
        QVERIFY(scene);
        scene->setProperty("animate", false);
        for (const QSize size : {QSize(525, 510), QSize(1547, 831), QSize(2400, 900)}) {
            scene->setSize(size);
            for (const auto *name : {"dayArtwork", "nightArtwork"}) {
                auto *art = scene->findChild<QQuickItem *>(name);
                QVERIFY(art);
                const auto frame = art->mapRectToItem(scene, art->boundingRect());
                QVERIFY(frame.left() <= 0.01 && frame.top() <= 0.01);
                QVERIFY(frame.right() >= size.width()-0.01);
                QVERIFY(frame.bottom() >= size.height()-0.01);
                if (size.width() >= 1500 && QByteArray(name) == "nightArtwork")
                    QVERIFY(art->mapToItem(scene, QPointF(1540, 102)).y() >= 20);
            }
            for (int step = 0; step < 24; ++step) {
                scene->setProperty("sailingPhase", step * 3.141592653589793 / 12);
                for (const auto *name : {"cpuSceneTag", "ramSceneTag", "networkSceneTag"}) {
                    auto *tag = scene->findChild<QQuickItem *>(name);
                    QVERIFY(tag);
                    const auto bounds = tag->mapRectToItem(scene, tag->boundingRect());
                    QVERIFY(bounds.left() >= 11.9 && bounds.top() >= 11.9);
                    QVERIFY(bounds.right() <= size.width()-11.9);
                    QVERIFY(bounds.bottom() <= size.height()-11.9);
                }
            }
        }
    }
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
        auto *life = scene->findChild<WorldLife *>("worldLife");
        QVERIFY(life);
        auto *art = scene->findChild<QQuickItem *>("nightArtwork");
        QVERIFY(art);
        // Moon's upper edge must have sky above it in the landscape viewport.
        QVERIFY(art->mapToItem(qobject_cast<QQuickItem *>(scene.data()), QPointF(1540, 102)).y() >= 20);
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
            const auto bottom = boat->mapToItem(qobject_cast<QQuickItem *>(scene.data()),
                                                QPointF(boat->width()/2, boat->height())).y();
            QVERIFY(bottom <= 830 - 70);
            previous = position;
        }
        const auto time = scene->property("sceneTime").toDouble();
        QCOMPARE(life->sceneTime(),time);
        QTest::qWait(120);
        QCOMPARE(scene->property("sceneTime").toDouble(), time);
        QCOMPARE(life->sceneTime(),time);
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
        if (window->rendererInterface()->graphicsApi() != QSGRendererInterface::Software) {
            // Verify rendered town pixels change, not just the animation clock.
            auto *art = window->findChild<QQuickItem *>("nightArtwork");
            QVERIFY(art);
            const qreal dpr = window->devicePixelRatio();
            const QRect town = QRectF(art->mapToScene(QPointF(650, 340))*dpr,
                                      art->mapToScene(QPointF(1000, 500))*dpr).toAlignedRect();
            scene->setProperty("animate", false);
            QTest::qWait(100);
            const QImage before = window->grabWindow().copy(town);
            QVERIFY(!before.isNull());
            QVERIFY(QMetaObject::invokeMethod(scene, "advanceScene", Q_ARG(QVariant, 2.0)));
            QTest::qWait(100);
            const QImage after = window->grabWindow().copy(town);
            QVERIFY(before != after);
            QTest::qWait(120);
            QCOMPARE(window->grabWindow().copy(town), after);
            if (!screenshotDir.isEmpty()) {
                QVERIFY(before.save(screenshotDir + "/town-before.png"));
                QVERIFY(after.save(screenshotDir + "/town-after.png"));
            }
            scene->setProperty("animate", true);
        }
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
        auto *dayTransition = window->findChild<QQuickItem *>("nightArtwork");
        QVERIFY(dayTransition);
        QTRY_VERIFY(dayTransition->opacity() < 0.001);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/day.png"));
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
