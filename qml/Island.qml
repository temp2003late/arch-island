import QtQuick
import QtQuick.Controls
import ArchIsland 1.0
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
        transformOrigin: Item.TopLeft
        x: (island.width-width*scale)/2
        // Keep the moon inside the crop even in an ultrawide window.
        property real visibleTop: Math.min(80, Math.max(0, (height-island.height/scale)/2))
        y: -visibleTop*scale
        // One complete panorama, with all effects positioned in the same world space.
        scale: Math.max(island.width/2000, island.height/1000)
        function tagX(preferred, pixels) {
            const left = (width - island.width/scale)/2
            return Math.max(left + 12/scale, Math.min(preferred, left + (island.width-pixels-12)/scale))
        }
        function tagY(preferred) {
            const top = visibleTop
            return Math.max(top + 12/scale, Math.min(preferred, top + (island.height-42)/scale))
        }
        Image {
            id: dayArt
            objectName: "dayArtwork"
            x: -200; y: 0; width: 2000; height: 1000
            source: "qrc:/assets/art/island-day-wide.png"
            smooth: true; mipmap: true
            layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software
            layer.effect: ShaderEffect {
                property real sceneTime: island.sceneTime
                property real darkness: 0
                fragmentShader: "qrc:/assets/shaders/living.frag.qsb"
            }
        }
        Image {
            id: nightArt
            objectName: "nightArtwork"
            x: -200; y: 0; width: 2000; height: 1000
            source: "qrc:/assets/art/island-night-wide.png"
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
            x: 499; y: 101; width: 120; height: 120
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
                x: 536+phase*48+index*3; y: 126-phase*95
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
                x: 560+Math.sin(index*4.3)*phase*32
                y: 170-phase*(35+island.heat*80)
                width: 1.5+index%2; height: width; radius: width
                color: "#ffb553"; opacity: Math.sin(Math.PI*phase)*island.heat

            }
        }
        Image {
            x: 1178; y: 220; width: 420; height: 118
            source: "qrc:/assets/beam.svg"; sourceSize: Qt.size(840,236)
            opacity: nightArt.opacity * (0.18 + 0.05*Math.sin(island.sceneTime*0.22 + 0.8))
            transformOrigin: Item.Left
            rotation: -1 + 15 * Math.sin(island.sceneTime * 0.22)

        }
        // Cargo is part of the painted harbor; RAM is shown by the tag and card.
        Item {
            id: boat
            objectName: "sailingBoat"
            x: Math.max(330, (world.width-island.width/world.scale)/2 + 185) + Math.min(165, island.width/world.scale*0.13) * Math.cos(island.sailingPhase)
            // Include the entire bobbing orbit plus a 72-screen-pixel water margin.
            property real routeCenterY: Math.min(745, world.visibleTop + island.height/world.scale - height - 36 - 72/world.scale)
            y: routeCenterY + 34 * Math.sin(island.sailingPhase) + 1.6 * Math.sin(island.sceneTime * 1.1)
            width: 190; height: 190
            // Convert the screen-space tangent into yaw for the elevated 3D camera.
            property real heading: Math.atan2(34 * Math.cos(island.sailingPhase) / Math.sin(24*Math.PI/180), -Math.min(165, island.width/world.scale*0.13) * Math.sin(island.sailingPhase)) * 180 / Math.PI
            Sailboat {
                objectName: "sailboatArtwork"
                anchors.fill: parent
                heading: boat.heading
                sceneTime: island.sceneTime
                darkness: nightArt.opacity
            }
        }
        Repeater {
            model: 5
            Item {
                required property int index
                x: 890 + 180*Math.sin(island.sceneTime*0.022 + index*0.35)
                y: 100 + index*10 + 10*Math.sin(island.sceneTime*0.065 + index)
                opacity: 0.55 - nightArt.opacity*0.35
                Rectangle { width: 5; height: 1; color: "#a4b8c5"; rotation: -15-16*Math.sin(island.sceneTime*2.4+index); transformOrigin: Item.Right }
                Rectangle { x: 5; width: 5; height: 1; color: "#a4b8c5"; rotation: 15+16*Math.sin(island.sceneTime*2.4+index); transformOrigin: Item.Left }
            }
        }
        Button {
            id: volcanoButton
            objectName: "volcanoButton"
            x: 453; y: 141; width: 216; height: 173
            text: "Open CPU processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 28; border.color: volcanoButton.visualFocus ? "#ffaf7b" : "transparent" }
            contentItem: Item {}
            onClicked: island.volcanoClicked()
            ToolTip.delay: 650; ToolTip.timeout: 2500
            ToolTip.visible: hovered; ToolTip.text: "Volcano · processes by CPU"
        }
        Button {
            id: portButton
            objectName: "portButton"
            x: 985; y: 625; width: 340; height: 155
            text: "Open memory processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 22; border.color: portButton.visualFocus ? "#83c7ff" : "transparent" }
            contentItem: Item {}
            onClicked: island.portClicked()
            ToolTip.delay: 650; ToolTip.timeout: 2500
            ToolTip.visible: hovered; ToolTip.text: "Harbor · processes by memory"
        }
        // Label sizes stay in screen pixels even as the art scales.
        SceneTag {
            objectName: "cpuSceneTag"
            x: world.tagX(646, width); y: world.tagY(126); scale: 1/world.scale; transformOrigin: Item.TopLeft
            text: "CPU"; accent: "#ff955e"; value: island.cpuLoad < 0 ? "—" : island.cpuLoad.toFixed(0)+"%"
            active: volcanoButton.hovered; onClicked: island.volcanoClicked()
        }
        SceneTag {
            id: ramTag
            objectName: "ramSceneTag"
            x: world.tagX(1170, width)
            y: world.tagY(776); scale: 1/world.scale; transformOrigin: Item.TopLeft
            text: "RAM"; accent: "#58b9ff"; value: island.ramRatio < 0 ? "—" : (island.ramRatio*100).toFixed(0)+"%"
            active: portButton.hovered; onClicked: island.portClicked()
        }
        SceneTag {
            objectName: "networkSceneTag"
            x: world.tagX(boat.x+215, width); y: world.tagY(boat.y+143); scale: 1/world.scale; transformOrigin: Item.TopLeft
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
