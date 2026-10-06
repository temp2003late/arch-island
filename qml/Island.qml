import QtQuick
import QtQuick.Controls
Item {
    id: island
    property bool night: false
    property bool animate: true
    property real cpuLoad: -1
    property real ramRatio: -1
    property real traffic: -1
    property bool artworkReady: nightArt.status === Image.Ready && dayArt.status === Image.Ready
    property real heat: cpuLoad < 0 ? 0 : Math.min(1, cpuLoad / 100)
    Behavior on heat { NumberAnimation { duration: island.animate ? 1400 : 0; easing.type: Easing.InOutSine } }
    // One accumulated clock: pauses preserve position; telemetry never resets a route.
    property real sceneTime: 0
    property real sailingPhase: 0.7
    property real sailingSpeed: 0.025
    function advanceScene(dt) {
        const target = 0.025 + Math.min(0.02, Math.log(1 + Math.max(0, traffic) / 4096) * 0.003)
        sailingSpeed += (target - sailingSpeed) * (1 - Math.exp(-dt / 4))
        sailingPhase += sailingSpeed * dt
        sceneTime += dt
    }
    Timer {
        interval: 33; repeat: true; running: island.animate
        property double previous: 0
        onRunningChanged: previous = Date.now()
        onTriggered: {
            const now = Date.now()
            island.advanceScene(Math.max(0, Math.min(0.1, (now - previous) / 1000)))
            previous = now
        }
    }
    signal volcanoClicked()
    signal portClicked()
    signal networkClicked()
    clip: true
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: island.night ? "#041329" : "#357ca7" }
            GradientStop { position: 0.65; color: "#082d43" }
            GradientStop { position: 1; color: "#061a2d" }
        }
    }
    Item {
        id: world
        width: 1600; height: 1000
        anchors.centerIn: parent
        // Extend only the outer sea, keeping the original pixels and landmark coordinates.
        // The height floor prevents gaps above/below in narrower windows.
        scale: Math.max(island.width/1728, island.height/1000)
        ShoreExtension { x: -64; rightEdge: false }
        ShoreExtension { x: 1600; rightEdge: true }
        Image {
            id: dayArt
            objectName: "dayArtwork"
            anchors.fill: parent
            source: "qrc:/assets/art/island-day.png"
            smooth: true; mipmap: true
            layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
            layer.effect: ShaderEffect {
                property real sceneTime: island.sceneTime
                property real darkness: island.night ? 1 : 0
                fragmentShader: "qrc:/assets/shaders/living.frag.qsb"
            }
        }
        Image {
            id: nightArt
            objectName: "nightArtwork"
            anchors.fill: parent
            source: "qrc:/assets/art/island-night.png"
            opacity: island.night ? 1 : 0
            smooth: true; mipmap: true
            layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
            layer.effect: ShaderEffect {
                property real sceneTime: island.sceneTime
                property real darkness: 1
                fragmentShader: "qrc:/assets/shaders/living.frag.qsb"
            }
            Behavior on opacity { NumberAnimation { duration: island.animate ? 1000 : 0; easing.type: Easing.InOutSine } }
        }
        // Soft atmospheric layers share the same clock as the sea and town.
        Image {
            x: 469; y: 30; width: 160; height: 160
            source: "qrc:/assets/glow.svg"; sourceSize: Qt.size(240,240)
            opacity: island.cpuLoad < 0 ? 0 : (0.16+island.heat*0.72) * (0.94 + 0.06*Math.sin(island.sceneTime*1.3))
            Behavior on opacity { NumberAnimation { duration: island.animate ? 800 : 0 } }
        }
        Repeater {
            model: 6
            Image {
                id: smoke
                required property int index
                property real phase: (island.sceneTime / 7 + index / 6) % 1
                x: 504+phase*48+index*3; y: 85-phase*95
                width: 48+phase*70; height: width
                source: "qrc:/assets/smoke.svg"; sourceSize: Qt.size(128,128)
                visible: island.cpuLoad >= 0
                opacity: Math.sin(Math.PI*phase)*(0.10+island.heat*0.5)

            }
        }
        Repeater {
            model: 7
            Rectangle {
                id: ember
                required property int index
                property real phase: (island.sceneTime / (2.8 + index*0.18) + index/7) % 1
                visible: island.cpuLoad > 30
                x: 546+Math.sin(index*4.3)*phase*32
                y: 111-phase*(35+island.heat*80)
                width: 1.5+index%2; height: width; radius: width
                color: "#ffb553"; opacity: Math.sin(Math.PI*phase)*island.heat

            }
        }
        Image {
            x: 1204; y: 157; width: 500; height: 146
            source: "qrc:/assets/beam.svg"; sourceSize: Qt.size(750,219)
            visible: island.night; opacity: 0.7
            transformOrigin: Item.Left
            rotation: -1 + 15 * Math.sin(island.sceneTime * 0.22)

        }
        // Cargo is part of the painted harbor; RAM is shown by the tag and card.
        Item {
            id: boat
            objectName: "sailingBoat"
            x: Math.max(330, (world.width-island.width/world.scale)/2 + 185) + Math.min(165, island.width/world.scale*0.13) * Math.cos(island.sailingPhase)
            property real routeCenterY: Math.min(745, (world.height + island.height/world.scale)/2 - height - 40)
            y: routeCenterY + 34 * Math.sin(island.sailingPhase) + 1.6 * Math.sin(island.sceneTime * 1.1)
            width: 190; height: 190
            property real heading: Math.atan2(34 * Math.cos(island.sailingPhase), -165 * Math.sin(island.sailingPhase)) * 180 / Math.PI
            Item {
                anchors.fill: parent
                rotation: 1.1 * Math.sin(island.sceneTime * 0.8)
                transform: Rotation { origin.x: 95; origin.y: 150; axis { x: 0; y: 1; z: 0 } angle: boat.heading }
                Image {
                    x: -58; y: 152; width: 175; height: 44
                    source: "qrc:/assets/wake.svg"; sourceSize: Qt.size(400,108)
                    opacity: 0.3 + 0.08 * Math.sin(island.sceneTime * 1.2)
                }
                Image {
                    objectName: "sailboatArtwork"
                    anchors.fill: parent; source: "qrc:/assets/art/sailboat.png"
                    smooth: true; mipmap: true
                }
            }
        }
        // Small independent routes bring the promenade and working harbor to life.
        Item {
            x: 1265; y: 527; width: 1; height: 63
            transformOrigin: Item.Top
            rotation: 2.5*Math.sin(island.sceneTime*0.55)
            Rectangle { width: 1; height: 59; color: island.night ? "#665c4b" : "#5b5446" }
            Rectangle { x: -2; y: 58; width: 5; height: 5; radius: 2; color: "#987448" }
        }
        Repeater {
            model: 9
            Item {
                required property int index
                property real walk: (1 - Math.cos(island.sceneTime * (0.025 + index*0.002) + index*1.7)) / 2
                x: index < 5 ? 891 + walk*215 : 1170 + walk*205
                y: index < 5 ? 601 + walk*38 + index*4 : 650 + walk*40 + (index-5)*5
                width: 4; height: 8
                Rectangle { x: 1; width: 2.5; height: 2.5; radius: 2; color: "#bb9874" }
                Rectangle { y: 2; width: 3.5; height: 5; radius: 1.5; color: index%2 ? "#7faaa9" : "#b69b73" }
                rotation: 2 * Math.sin(island.sceneTime*3 + index)
            }
        }
        Repeater {
            model: 12
            Image {
                required property int index
                property var lamps: [[480,270],[571,305],[747,354],[790,410],[899,447],[974,482],[720,540],[894,582],[1021,609],[1166,643],[1280,669],[1390,694]]
                x: lamps[index][0]-14; y: lamps[index][1]-14
                width: 28; height: 28
                source: "qrc:/assets/glow.svg"
                opacity: (island.night ? 0.2 : 0.045) * (0.8 + 0.2*Math.sin(island.sceneTime*0.65 + index*2.3))
            }
        }
        Repeater {
            model: 5
            Item {
                required property int index
                x: 890 + 180*Math.sin(island.sceneTime*0.022 + index*0.35)
                y: 100 + index*10 + 10*Math.sin(island.sceneTime*0.065 + index)
                opacity: island.night ? 0.3 : 0.6
                Rectangle { width: 5; height: 1; color: "#a4b8c5"; rotation: -15-16*Math.sin(island.sceneTime*2.4+index); transformOrigin: Item.Right }
                Rectangle { x: 5; width: 5; height: 1; color: "#a4b8c5"; rotation: 15+16*Math.sin(island.sceneTime*2.4+index); transformOrigin: Item.Left }
            }
        }
        Button {
            id: volcanoButton
            objectName: "volcanoButton"
            x: 435; y: 70; width: 230; height: 207
            text: "Open CPU processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 28; border.color: volcanoButton.activeFocus ? "#ffaf7b" : "transparent" }
            contentItem: Item {}
            onClicked: island.volcanoClicked()
            ToolTip.visible: hovered; ToolTip.text: "Volcano · processes by CPU"
        }
        Button {
            id: portButton
            objectName: "portButton"
            x: 965; y: 587; width: 264; height: 138
            text: "Open memory processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 22; border.color: portButton.activeFocus ? "#83c7ff" : "transparent" }
            contentItem: Item {}
            onClicked: island.portClicked()
            ToolTip.visible: hovered; ToolTip.text: "Harbor · processes by memory"
        }
        Rectangle { x: 592; y: 146; width: 82; height: 1.3; rotation: -35; transformOrigin: Item.Left; color: "#a8c8e1"; opacity: 0.65 }
        Rectangle { x: 1149; y: 704; width: 68; height: 1.3; rotation: 22; transformOrigin: Item.Left; color: "#a8c8e1"; opacity: 0.65 }
        Rectangle { x: boat.x+205; y: boat.y+167; width: 41; height: 1.3; rotation: -26; transformOrigin: Item.Left; color: "#a8c8e1"; opacity: 0.65 }
        // Label sizes stay in screen pixels even as the art scales.
        SceneTag {
            x: 646; y: 81; scale: 1/world.scale; transformOrigin: Item.TopLeft
            text: "CPU"; accent: "#ff955e"; value: island.cpuLoad < 0 ? "—" : island.cpuLoad.toFixed(0)+"%"
            active: volcanoButton.hovered; onClicked: island.volcanoClicked()
        }
        SceneTag {
            id: ramTag
            x: Math.min(1205,(island.width-width-12-(island.width-world.width*world.scale)/2)/world.scale)
            y: 708; scale: 1/world.scale; transformOrigin: Item.TopLeft
            text: "RAM"; accent: "#58b9ff"; value: island.ramRatio < 0 ? "—" : (island.ramRatio*100).toFixed(0)+"%"
            active: portButton.hovered; onClicked: island.portClicked()
        }
        SceneTag {
            x: boat.x+237; y: boat.y+133; scale: 1/world.scale; transformOrigin: Item.TopLeft
            text: "Network"; accent: "#39ddb8"
            onClicked: island.networkClicked()
        }
    }
    // Edge shading helps the scene sit inside the application chrome.
    Rectangle {
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; height: 38
        gradient: Gradient { GradientStop { position: 0; color: "#75051120" } GradientStop { position: 1; color: "transparent" } }
    }
    Rectangle {
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 48
        gradient: Gradient { GradientStop { position: 0; color: "transparent" } GradientStop { position: 1; color: "#b3051120" } }
    }
    component ShoreExtension: Item {
        property bool rightEdge: false
        width: 64; height: 1000
        clip: true
        // Reflect a narrow strip of the existing sea at its exact boundary.
        // These share the original image textures: no resized or recompressed assets.
        Image {
            x: parent.rightEdge ? 0 : 64-1600
            width: 1600; height: 1000
            source: dayArt.source
            mirror: true; smooth: true; mipmap: true
        }
        Image {
            x: parent.rightEdge ? 0 : 64-1600
            width: 1600; height: 1000
            source: nightArt.source
            opacity: nightArt.opacity
            mirror: true; smooth: true; mipmap: true
        }
    }
    component SceneTag: Button {
        id: tag
        property color accent: "#5fbdff"
        property string value: ""
        property bool active: false
        hoverEnabled: true
        width: tagContent.implicitWidth+22; height: 30
        background: Rectangle {
            radius: 11; color: tag.hovered || tag.active ? "#ed132b40" : "#de071728"
            border.color: tag.hovered || tag.active ? tag.accent : "#486176"
        }
        contentItem: Row {
            id: tagContent
            spacing: 7
            Rectangle { width: 6; height: 6; radius: 3; color: tag.accent; anchors.verticalCenter: parent.verticalCenter }
            Text { text: tag.text; color: "#eef6ff"; font.pixelSize: 11; font.weight: Font.Medium; anchors.verticalCenter: parent.verticalCenter }
            Text { visible: tag.value.length > 0; text: tag.value; color: tag.accent; font.pixelSize: 10; anchors.verticalCenter: parent.verticalCenter }
        }
        leftPadding: 11; rightPadding: 11
    }
}
