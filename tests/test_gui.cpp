#include <QtTest>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickStyle>
#include <QPainter>
#include <limits>
#include <optional>
#include "sailboat.h"
#include "monitor.h"
#include "charts.h"
#include "worldlife.h"
#include "sceneclock.h"
// A world-space landmark can be partly or completely outside a cover-cropped
// viewport. Click its visible area; never send events beyond the actual scene.
static std::optional<QPoint> visibleClickPoint(QQuickItem *item, QQuickItem *viewport,
                                              QQuickWindow *window) {
    const QRectF visible=item->mapRectToScene(item->boundingRect())
        .intersected(viewport->mapRectToScene(viewport->boundingRect()))
        .intersected(QRectF(QPointF(0,0),QSizeF(window->size())));
    if(visible.width()<4 || visible.height()<4) return std::nullopt;
    return visible.center().toPoint();
}
class GuiTest : public QObject {
    Q_OBJECT
private slots:
    void monotonicClockPauseAndStall() {
        SceneClock clock;
        QSignalSpy steps(&clock,&SceneClock::stepped);
        clock.setRunning(true);
        QTRY_VERIFY(steps.size()>=2);
        QTest::qSleep(400); // Simulate a blocked render/event loop.
        QCoreApplication::processEvents();
        for (const auto &step:steps) {
            QVERIFY(step[0].toDouble()>0);
            QVERIFY(step[0].toDouble()<=.1);
        }
        clock.setRunning(false);
        const auto count=steps.size();
        QTest::qWait(140);
        QCOMPARE(steps.size(),count);
        clock.setRunning(true);
        QTRY_VERIFY(steps.size()>count);
        QVERIFY(steps.last()[0].toDouble()<.1);
    }
    void sharedMasksProtectArchitecture() {
        const QImage mask(":/assets/art/scene-mask.png");
        QCOMPARE(mask.size(),QSize(1920,1080));
        // Half-resolution native coordinates: residence, church, lighthouse,
        // decks, foreground rocks, and both ends of the island silhouette.
        for (const QPoint point:{QPoint(550,140),QPoint(1040,350),QPoint(1670,250),
                                QPoint(1450,650),QPoint(200,650),QPoint(200,900),
                                QPoint(1650,850),QPoint(1820,470)})
            QCOMPARE(qRed(mask.pixel(point)),0);
        for (const QPoint point:{QPoint(950,900),QPoint(1150,800),QPoint(60,300)})
            QCOMPARE(qRed(mask.pixel(point)),255);
        QCOMPARE(qBlue(mask.pixel(550,140)),0);
        const QImage day(":/assets/art/island.png"), night(":/assets/art/island_night.png");
        QCOMPARE(day.size(),QSize(3840,2160));
        QCOMPARE(night.size(),day.size());
    }
    void lightingTransitionKeepsState() {
        QQmlEngine engine;
        QQmlComponent component(&engine,QUrl("qrc:/qml/Island.qml"));
        QScopedPointer<QObject> scene(component.create());
        QVERIFY2(scene,qPrintable(component.errorString()));
        scene->setProperty("animate",false);
        scene->setProperty("width",1600);
        scene->setProperty("height",1000);
        auto *boat=scene->findChild<QQuickItem *>("sailingBoat");
        auto *art=scene->findChild<QQuickItem *>("nightArtwork");
        QVERIFY(boat && art);
        const double time=scene->property("sceneTime").toDouble();
        const QPointF position=boat->position();
        scene->setProperty("night",true);
        QTest::qWait(350);
        QVERIFY(art->opacity()>0 && art->opacity()<1);
        const double current=art->opacity();
        scene->setProperty("night",false);
        QVERIFY(std::abs(art->opacity()-current)<.002);
        scene->setProperty("cpuLoad",81);
        scene->setProperty("traffic",1e8);
        scene->setProperty("width",1800);
        QTest::qWait(160);
        QCOMPARE(scene->property("sceneTime").toDouble(),time);
        QCOMPARE(boat->position(),position);
        QTRY_VERIFY_WITH_TIMEOUT(art->opacity()<.001,5000);
        // A long period of hidden/paused time has no accumulated physics debt.
        scene->setProperty("animate",true);
        QTest::qWait(110);
        QVERIFY(scene->property("sceneTime").toDouble()>time);
        QVERIFY(QLineF(position,boat->position()).length()<.5);
    }
    void residentsStayOnWalkways() {
        // Independently traced pavement/deck boundaries in panorama world space.
        // These are wider areas, not curves generated from the renderer's routes.
        const QPolygonF walkable[] = {
            {{372,487},{380,483},{415,519},{410,527},{394,514}},
            {{560,446},{670,419},{680,430},{572,458}},
            {{799,348},{886,351},{886,367},{799,364}},
            {{1040,497},{1132,521},{1130,533},{1040,511}},
            {{1154,530},{1226,548},{1225,560},{1153,541}},
            {{111,522},{185,533},{185,546},{111,538}}
        };
        // Cover many return trips for every resident, not just the initial frame.
        for (int resident=0; resident<WorldLife::ResidentCount; ++resident) {
            auto previous = WorldLife::residentPosition(resident,0);
            for (int step=0; step<6000; ++step) {
                const auto point = WorldLife::residentPosition(resident,step*.1);
                QVERIFY2(walkable[resident%6].containsPoint(point,Qt::OddEvenFill),
                         qPrintable(QString("Resident %1 left the walkway at %2s: (%3, %4)")
                                    .arg(resident).arg(step*.1).arg(point.x()).arg(point.y())));
                QVERIFY(QLineF(previous,point).length()<2);
                previous = point;
            }
        }
    }
    void worldLifeAnimation() {
        QTest::failOnWarning(QRegularExpression("^(QColor|QPainter)::"));
        WorldLife life;
        life.setSize(QSizeF(1600,900));
        auto frame = [&life] {
            QImage image(1600,900,QImage::Format_ARGB32_Premultiplied);
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
        for (const QSize size : {QSize(504, 486), QSize(525, 510), QSize(1547, 831), QSize(2400, 900)}) {
            scene->setSize(size);
            for (const auto *name : {"dayArtwork", "nightArtwork"}) {
                auto *art = scene->findChild<QQuickItem *>(name);
                QVERIFY(art);
                const auto frame = art->mapRectToItem(scene, art->boundingRect());
                QVERIFY(frame.left() <= 0.01 && frame.top() <= 0.01);
                QVERIFY(frame.right() >= size.width()-0.01);
                QVERIFY(frame.bottom() >= size.height()-0.01);
                // Artwork keeps its 16:9 geometry at every viewport size.
                QCOMPARE(art->width()/art->height(), 16.0/9.0);
                if (size.width() >= 1500)
                    QVERIFY(art->mapToItem(scene, QPointF(475, 70)).y() >= 20);
            }
            for (int step = 0; step < 24; ++step) {
                scene->setProperty("sailingPhase", step * 3.141592653589793 / 12);
                for (const auto *name : {"cpuSceneTag", "ramSceneTag", "networkSceneTag", "storageSceneTag", "alertsSceneTag"}) {
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
        auto *sprite=scene->findChild<QQuickItem *>("sailboatArtwork");
        QVERIFY(sprite);
        QCOMPARE(sprite->property("source").toUrl(),QUrl("qrc:/assets/art/sailboat.png"));
        QTRY_COMPARE(sprite->property("status").toInt(),1); // Image.Ready
        auto *life = scene->findChild<WorldLife *>("worldLife");
        QVERIFY(life);
        auto *art = scene->findChild<QQuickItem *>("nightArtwork");
        QVERIFY(art);
        // Preserve the summit roof below the top edge.
        QVERIFY(art->mapToItem(qobject_cast<QQuickItem *>(scene.data()), QPointF(475, 70)).y() >= 20);
        QPointF previous = boat->position();
        // Several full circuits, including idle, bursts and unavailable metrics.
        for (int i = 0; i < 7200; ++i) {
            if (i % 300 == 0)
                scene->setProperty("traffic", i % 900 == 0 ? -1.0 : i % 600 == 0 ? 0.0 : 1e9);
            QVERIFY(QMetaObject::invokeMethod(scene.data(), "advanceScene", Q_ARG(QVariant, 0.1)));
            const auto position = boat->position();
            QVERIFY(QLineF(previous, position).length() < 1.0);
            QVERIFY(position.x() >= 775 && position.x() <= 785);
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
    void nightThemeCoversChrome() {
        QTemporaryDir dir;
        Settings settings(dir.filePath("settings.ini"));
        settings.setMode("night");
        Monitor monitor(&settings);
        QQmlApplicationEngine engine;
        QStringList warnings;
        connect(&engine,&QQmlEngine::warnings,this,[&](const QList<QQmlError> &errors) {
            for(const auto &error:errors) warnings << error.toString();
        });
        engine.rootContext()->setContextProperty("preferences",&settings);
        engine.rootContext()->setContextProperty("monitor",&monitor);
        engine.load(QUrl("qrc:/qml/Main.qml"));
        QCOMPARE(engine.rootObjects().size(),1);
        auto *window=qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto *scene=window->findChild<QQuickItem *>("islandScene");
        auto *headerPanel=window->findChild<QQuickItem *>("headerPanel");
        QVERIFY(scene && headerPanel);
        for(const bool night:{true,false,true}) {
            const double before=scene->property("lightingBlend").toDouble();
            settings.setMode(night ? "night" : "day");
            QTRY_COMPARE(monitor.night(),night);
            if(before!=(night ? 1.0 : 0.0)) {
                QTest::qWait(600);
                const double progress=scene->property("lightingBlend").toDouble();
                QVERIFY(progress>0 && progress<1);
                const QColor panel=headerPanel->property("color").value<QColor>();
                // The stepped panel must be between its endpoints and use
                // the very same progress as the panorama crossfade.
                QVERIFY(std::abs(panel.red()-(241+(13-241)*progress))<=1);
                QVERIFY(std::abs(panel.green()-(237+(28-237)*progress))<=1);
                QVERIFY(std::abs(panel.blue()-(229+(43-229)*progress))<=1);
                if(!night) {
                    // Reverse mid-transition: neither color nor scene jumps.
                    settings.setMode("night");
                    QTRY_VERIFY(monitor.night());
                    QCOMPARE(headerPanel->property("color").value<QColor>(),panel);
                    QTest::qWait(120);
                    settings.setMode("day");
                    QTRY_VERIFY(!monitor.night());
                }
            }
            QTRY_COMPARE_WITH_TIMEOUT(scene->property("lightingBlend").toDouble(),night ? 1.0 : 0.0,5000);
            QTest::qWait(120);
            const auto image=window->grabWindow();
            QVERIFY(!image.isNull());
            for(const auto *name:{"headerPanel","metricsPanel","footerPanel","cpuCard","storageCard"}) {
                const auto *item=window->findChild<QQuickItem *>(name);
                QVERIFY(item);
                const auto point=(item->mapToScene(QPointF(6,6))*window->devicePixelRatio()).toPoint();
                QVERIFY(image.rect().contains(point));
                const int brightness=qGray(image.pixel(point));
                QVERIFY2(night ? brightness<85 : brightness>200,name);
            }
            // The logo's corner must show the actual panel color, with no
            // white or gray backing left in either lighting mode.
            auto *mark=window->findChild<QQuickItem *>("headerBrandMark");
            auto *header=window->findChild<QQuickItem *>("headerPanel");
            QVERIFY(mark && header);
            const auto point=(mark->mapToScene(QPointF(1,1))*window->devicePixelRatio()).toPoint();
            const QColor expected=header->property("color").value<QColor>();
            QQmlExpression tooltipBase(engine.rootContext(),window,"palette.toolTipBase");
            QCOMPARE(tooltipBase.evaluate().value<QColor>(),expected);
            QVERIFY(!tooltipBase.hasError());
            const QColor actual=image.pixelColor(point);
            QVERIFY(std::abs(actual.red()-expected.red())<=3);
            QVERIFY(std::abs(actual.green()-expected.green())<=3);
            QVERIFY(std::abs(actual.blue()-expected.blue())<=3);
        }
        QVERIFY2(warnings.isEmpty(),qPrintable(warnings.join('\n')));
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
        QVERIFY(monitor.processCount() > 0);
        QVERIFY(monitor.storageTotal() > 0);
        QVERIFY(monitor.storageUsed() >= 0);
        QVERIFY(monitor.storageUsed() <= monitor.storageTotal());
        QVERIFY(!monitor.storageFormat().isEmpty());
        QVERIFY(monitor.uptime() >= 0);
        QVERIFY(!monitor.hostName().isEmpty());
        for (const auto &entry : {qMakePair("storageNav", "storageDialog"), qMakePair("alertsNav", "alertsDialog"), qMakePair("networkNav", "networkDialog"),
                                  qMakePair("storageSceneTag", "storageDialog"), qMakePair("alertsSceneTag", "alertsDialog"),
                                  qMakePair("networkSceneTag", "networkDialog")}) {
            auto *nav = window->findChild<QQuickItem *>(entry.first);
            auto *dialog = window->findChild<QObject *>(entry.second);
            QVERIFY(nav); QVERIFY(dialog);
            QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
                nav->mapToScene(QPointF(nav->width()/2, nav->height()/2)).toPoint());
            QTRY_VERIFY(dialog->property("visible").toBool());
            QVERIFY(QMetaObject::invokeMethod(dialog, "close"));
            QTRY_VERIFY(!dialog->property("visible").toBool());
            QTest::qWait(200);
        }
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
            // Keep the scene independent of live CPU samples during pixel checks.
            scene->setProperty("animate",false);
            QQmlExpression freeze(engine.rootContext(),scene,"cpuLoad = 50");
            freeze.evaluate();
            QVERIFY2(!freeze.hasError(),qPrintable(freeze.error().toString()));
            QTest::qWait(100);
            const QImage fullBefore = window->grabWindow();
            const QImage before = fullBefore.copy(town);
            QVERIFY(!before.isNull());
            QVERIFY(QMetaObject::invokeMethod(scene, "advanceScene", Q_ARG(QVariant, 2.0)));
            QTest::qWait(100);
            const QImage fullAfter = window->grabWindow();
            const QImage after = fullAfter.copy(town);
            QVERIFY(before != after);
            const auto artworkRect=[&](QRectF native) {
                const QPointF a(native.left()*1600/3840,native.top()*900/2160);
                const QPointF b(native.right()*1600/3840,native.bottom()*900/2160);
                return QRectF(art->mapToScene(a)*dpr,art->mapToScene(b)*dpr).toAlignedRect();
            };
            const QRect sea=artworkRect(QRectF(2200,1850,180,90));
            QVERIFY(fullBefore.rect().contains(sea));
            QVERIFY(fullBefore.copy(sea)!=fullAfter.copy(sea));
            // A solid roof stays pixel-identical while neighboring waves move.
            const QRect roof=artworkRect(QRectF(2010,663,14,7));
            QVERIFY(fullBefore.rect().contains(roof));
            QCOMPARE(fullBefore.copy(roof),fullAfter.copy(roof));
            QTest::qWait(120);
            QCOMPARE(window->grabWindow().copy(town), after);
            if (!screenshotDir.isEmpty()) {
                QVERIFY(before.save(screenshotDir + "/town-before.png"));
                QVERIFY(after.save(screenshotDir + "/town-after.png"));
            }
            scene->setProperty("animate", true);
            QQmlExpression restore(engine.rootContext(),scene,
                                  "cpuLoad = Qt.binding(function() { return monitor.cpu; })");
            restore.evaluate();
            QVERIFY2(!restore.hasError(),qPrintable(restore.error().toString()));
        }
        auto *volcano = window->findChild<QQuickItem *>("volcanoButton");
        QVERIFY(volcano);
        const auto summitPoint=visibleClickPoint(volcano,scene,window);
        auto *cpuTag=window->findChild<QQuickItem *>("cpuSceneTag");
        QVERIFY(cpuTag);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            summitPoint.value_or(cpuTag->mapToScene(QPointF(cpuTag->width()/2,cpuTag->height()/2)).toPoint()));
        QTRY_VERIFY(monitor.processesOpen()); QVERIFY(!monitor.sortByMemory());
        QTRY_VERIFY_WITH_TIMEOUT(monitor.processes()->rowCount() > 0, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(monitor.processes()->data(monitor.processes()->index(0), ProcessModel::CpuRole).toDouble() >= 0, 5000);
        QTest::qWait(400);
        if (!screenshotDir.isEmpty()) QVERIFY(window->grabWindow().save(screenshotDir + "/processes.png"));
        auto *drawer = window->findChild<QObject *>("processesDrawer"); QVERIFY(drawer);
        QVERIFY(QMetaObject::invokeMethod(drawer,"close"));
        QTRY_VERIFY(!monitor.processesOpen());
        auto *port = window->findChild<QQuickItem *>("portButton"); QVERIFY(port);
        auto *processTag=window->findChild<QQuickItem *>("ramSceneTag");
        QVERIFY(processTag);
        const auto portPoint=visibleClickPoint(port,scene,window);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
            portPoint.value_or(processTag->mapToScene(QPointF(processTag->width()/2,processTag->height()/2)).toPoint()));
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
        if (!screenshotDir.isEmpty()) {
            QTest::mouseMove(window,QPoint(window->width()-8,8));
            QTest::qWait(180);
            QVERIFY(window->grabWindow().save(screenshotDir + "/day.png"));
        }
        settings.setReducedMotion(false);
        window->resize(800,640); QTest::qWait(200);
        QVERIFY(scene->width() > 0 && scene->height() > 0);
        // Both process views remain reachable from native badges, including
        // tall/narrow crops where a painted landmark is entirely offscreen.
        for(const QSize size:{QSize(800,640),QSize(800,1100)}) {
            window->resize(size); QTest::qWait(150);
            for(const auto &action:{qMakePair(cpuTag,false),qMakePair(processTag,true)}) {
                const auto point=visibleClickPoint(action.first,scene,window);
                QVERIFY(point);
                QTest::mouseClick(window,Qt::LeftButton,Qt::NoModifier,*point);
                QTRY_VERIFY(monitor.processesOpen());
                QCOMPARE(monitor.sortByMemory(),action.second);
                QVERIFY(QMetaObject::invokeMethod(drawer,"close"));
                QTRY_VERIFY(!monitor.processesOpen());
                QTest::qWait(200);
            }
        }
        window->resize(800,640); QTest::qWait(150);
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
