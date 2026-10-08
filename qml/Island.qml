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
    // One prewarmed, monotonic simulation clock shared by every lighting mode.
    property real sceneTime: 143.7
    property real sailingPhase: 0.7
    property real sailingSpeed: 0.025
    function advanceScene(dt) {
        if (!Number.isFinite(dt) || dt <= 0) return
        // Kept as a continuous public phase for integrations; boats stay moored.
        sailingPhase += sailingSpeed * dt
        sceneTime += dt
    }
    SceneClock {
        running: island.animate
        onStepped: seconds => island.advanceScene(seconds)
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
            id: sceneMask
            source: "qrc:/assets/art/scene-mask.png"
            visible: false; smooth: true
        }
        Image {
            id: dayArt
            objectName: "dayArtwork"
            x: -200; y: 0; width: 2000; height: 1000
            source: "qrc:/assets/art/island-day-wide.png"
            smooth: true; mipmap: true
            visible: GraphicsInfo.api === GraphicsInfo.Software
        }
        Image {
            id: nightArt
            objectName: "nightArtwork"
            x: -200; y: 0; width: 2000; height: 1000
            source: "qrc:/assets/art/island-night-wide.png"
            opacity: island.night ? 1 : 0
            smooth: true; mipmap: true
            visible: GraphicsInfo.api === GraphicsInfo.Software
            Behavior on opacity { NumberAnimation { duration: 4000; easing.type: Easing.InOutSine } }
        }
        ShaderEffect {
            objectName: "livingSurface"
            x: -200; y: 0; width: 2000; height: 1000
            visible: GraphicsInfo.api !== GraphicsInfo.Software
            property var source: dayArt
            property var nightSource: nightArt
            property var maskSource: sceneMask
            property real sceneTime: island.sceneTime
            property real darkness: nightArt.opacity
            property real heat: island.heat
            fragmentShader: "qrc:/assets/shaders/living.frag.qsb"
        }
        WorldLife {
            objectName: "worldLife"
            width: 1600; height: 1000
            sceneTime: island.sceneTime
            darkness: nightArt.opacity
            textureSize: Qt.size(Math.max(1, Math.min(1600, width*world.scale)), Math.max(1, Math.min(1000, height*world.scale)))
        }
        // Irregular, analytically aged plumes are present on the first frame.
        Image {
            x: 529; y: 142; width: 80; height: 50
            source: "qrc:/assets/glow.svg"; sourceSize: Qt.size(160,100)
            opacity: (0.07 + nightArt.opacity*0.11) * (0.91+0.06*Math.sin(island.sceneTime*.173)+0.03*Math.sin(island.sceneTime*.317))
        }
        Repeater {
            model: 13
            Image {
                required property int index
                property real lifetime: 13.1 + index*.731
                property real phase: (island.sceneTime / lifetime + index*.61803398875) % 1
                property real wind: .58*Math.sin(island.sceneTime*.173)+.27*Math.sin(island.sceneTime*.317+1.2)+.15*Math.sin(island.sceneTime*.071+2.4)
                x: 538 + phase*(24+index*1.3) + wind*phase*12
                y: 151 - phase*(80+index*2.7)
                width: 30 + phase*(52+index*1.1); height: width*(.8+index%3*.11)
                source: "qrc:/assets/smoke.svg"; sourceSize: Qt.size(96,96)
                rotation: index*47 + phase*13
                opacity: Math.pow(Math.sin(Math.PI*phase),2)*(.045+.025*(1-nightArt.opacity))
            }
        }
        Repeater {
            model: 5
            Rectangle {
                required property int index
                property real phase: (island.sceneTime / (7.7 + index*1.137) + index*.61803398875) % 1
                x: 562 + Math.sin(index*4.3)*phase*16 + phase*8*Math.sin(island.sceneTime*.173)
                y: 173-phase*(27+index*4)
                width: 1.1+index%2*.4; height: width; radius: width
                color: "#ffb553"
                opacity: Math.pow(Math.sin(Math.PI*phase),3)*(.15+.28*nightArt.opacity)
            }
        }
        // Beam points out to open water; negative projection is occluded by the
        // tower/island. Its room rotates throughout daytime too.
        Image {
            x: 1178; y: 269; width: 340; height: 30
            source: "qrc:/assets/beam.svg"; sourceSize: Qt.size(680,60)
            property real azimuth: island.sceneTime*.137
            opacity: (0.004+nightArt.opacity*.038)*Math.pow(Math.max(0,Math.cos(azimuth)),2)
            transformOrigin: Item.Left
            scale: .25+.75*Math.abs(Math.cos(azimuth))
            rotation: 4+9*Math.sin(azimuth)
        }
        Image {
            x: 1169; y: 264; width: 21; height: 18
            source: "qrc:/assets/glow.svg"
            opacity: (.05+.14*nightArt.opacity)*(.65+.35*Math.cos(island.sceneTime*.137))
        }
        // Cargo is part of the painted harbor; RAM is shown by the tag and card.
        Item {
            id: boat
            objectName: "sailingBoat"
            x: Math.max(330, (world.width-island.width/world.scale)/2 + 20/world.scale) + 2.2*Math.sin(island.sceneTime*.219) + .7*Math.sin(island.sceneTime*.373+1.4)
            property real routeCenterY: Math.min(720, world.visibleTop + island.height/world.scale - height - 70/world.scale)
            y: routeCenterY + 1.3*Math.sin(island.sceneTime*.713) + .55*Math.sin(island.sceneTime*1.137+2.1)
            width: 190; height: 190
            property real heading: -18 + .7*Math.sin(island.sceneTime*.173)
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
