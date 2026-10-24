import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Shapes
import ArchIsland 1.0
Item {
    id: island
    property bool night: false
    property real transitionProgress: night ? 1 : 0
    // Irregular plateaus, short jumps and two small signal rebounds.
    // A single reversible driver keeps the image and interface in lockstep.
    function digitalBlend(progress) {
        if (!animate) return progress
        const times = [0, .10, .14, .27, .30, .34, .45, .50, .58, .62, .76, .80, .90, .94, 1]
        const levels = [0, 0, .18, .18, .13, .13, .48, .48, .44, .44, .80, .80, .92, .92, 1]
        for (let i = 1; i < times.length; ++i) {
            if (progress <= times[i]) {
                const t = (progress-times[i-1])/(times[i]-times[i-1])
                return levels[i-1] + (levels[i]-levels[i-1])*Math.max(0,t)
            }
        }
        return 1
    }
    readonly property real lightingBlend: digitalBlend(transitionProgress)
    readonly property real transitionGlitch: animate && (
        (transitionProgress > .10 && transitionProgress < .16) ||
        (transitionProgress > .45 && transitionProgress < .50) ||
        (transitionProgress > .76 && transitionProgress < .80)) ? 1 : 0
    Behavior on transitionProgress {
        NumberAnimation { duration: 1000; easing.type: Easing.Linear }
    }
    property bool animate: true
    property real cpuLoad: -1
    property real ramRatio: -1
    property real traffic: -1
    property var cpuHistory: []
    property var memoryHistory: []
    property string networkValue: ""
    property string networkOutValue: ""
    property real storageRatio: -1
    property int processCount: -1
    property int alertCount: 0
    property var receiveHistory: []
    property bool artworkReady: nightArt.status === Image.Ready && dayArt.status === Image.Ready
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
    signal storageClicked()
    signal alertsClicked()
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
        width: 1600; height: 900
        transformOrigin: Item.TopLeft
        x: (island.width-width*scale)/2
        // Preserve the summit and lighthouse when a wide viewport crops the sky.
        property real visibleTop: Math.min(40, Math.max(0, (height-island.height/scale)/2))
        y: -visibleTop*scale
        // One complete panorama, with all effects positioned in the same world space.
        scale: Math.max(island.width/1600, island.height/900)
        function tagX(preferred, pixels) {
            const left = (width - island.width/scale)/2
            return Math.max(left + 12/scale, Math.min(preferred, left + (island.width-pixels-12)/scale))
        }
        function tagY(preferred) {
            const top = visibleTop
            return Math.max(top + 12/scale, Math.min(preferred, top + (island.height-70)/scale))
        }
        Image {
            id: sceneMask
            source: "qrc:/assets/art/scene-mask.png"
            visible: false; smooth: true
        }
        Image {
            id: dayArt
            objectName: "dayArtwork"
            x: 0; y: 0; width: 1600; height: 900
            source: "qrc:/assets/art/island.png"
            smooth: true; mipmap: true
            visible: GraphicsInfo.api === GraphicsInfo.Software
        }
        Image {
            id: nightArt
            objectName: "nightArtwork"
            x: 0; y: 0; width: 1600; height: 900
            source: "qrc:/assets/art/island_night.png"
            opacity: island.lightingBlend
            smooth: true; mipmap: true
            visible: GraphicsInfo.api === GraphicsInfo.Software
        }
        ShaderEffect {
            objectName: "livingSurface"
            x: 0; y: 0; width: 1600; height: 900
            visible: GraphicsInfo.api !== GraphicsInfo.Software
            property var source: dayArt
            property var nightSource: nightArt
            property var maskSource: sceneMask
            property real sceneTime: island.sceneTime
            property real darkness: nightArt.opacity
            fragmentShader: "qrc:/assets/shaders/living.frag.qsb"
        }
        WorldLife {
            objectName: "worldLife"
            width: 1600; height: 900
            sceneTime: island.sceneTime
            darkness: nightArt.opacity
            textureSize: Qt.size(Math.max(1, Math.min(1600, width*world.scale)), Math.max(1, Math.min(900, height*world.scale)))
        }
        // Beam points out to open water; negative projection is occluded by the
        // tower/island. Its room rotates throughout daytime too.
        Image {
            x: 1392; y: 172; width: 340; height: 30
            source: "qrc:/assets/beam.svg"; sourceSize: Qt.size(680,60)
            property real azimuth: island.sceneTime*.137
            opacity: (0.13+nightArt.opacity*.25)*Math.pow(Math.max(0,Math.cos(azimuth)),2)
            transformOrigin: Item.Left
            scale: .25+.75*Math.abs(Math.cos(azimuth))
            rotation: 4+9*Math.sin(azimuth)
        }
        Image {
            x: 1382; y: 164; width: 21; height: 18
            source: "qrc:/assets/glow.svg"
            opacity: (.3+.3*nightArt.opacity)*(.65+.35*Math.cos(island.sceneTime*.137))
        }
        // Cargo is part of the painted harbor; RAM is shown by the tag and card.
        Item {
            id: boat
            objectName: "sailingBoat"
            x: 780 + 2.2*Math.sin(island.sceneTime*.219) + .7*Math.sin(island.sceneTime*.373+1.4)
            property real routeCenterY: Math.min(610, world.visibleTop + island.height/world.scale - height - 70/world.scale)
            y: routeCenterY + 1.3*Math.sin(island.sceneTime*.713) + .55*Math.sin(island.sceneTime*1.137+2.1)
            width: 160; height: 160
            rotation: .45*Math.sin(island.sceneTime*.713)
            transformOrigin: Item.Bottom
            Image {
                objectName: "sailboatArtwork"
                anchors.fill: parent
                source: "qrc:/assets/art/sailboat.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
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
            x: 375; y: 65; width: 230; height: 130
            text: "Open CPU processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 28; border.color: volcanoButton.visualFocus ? "#ffaf7b" : "transparent" }
            contentItem: Item {}
            onClicked: island.volcanoClicked()
            ToolTip.delay: 650; ToolTip.timeout: 2500
            ToolTip.visible: hovered; ToolTip.text: "Summit · processes by CPU"
        }
        Button {
            id: portButton
            objectName: "portButton"
            x: 1020; y: 470; width: 340; height: 125
            text: "Open memory processes"
            Accessible.name: text
            hoverEnabled: true
            background: Rectangle { color: "transparent"; radius: 22; border.color: portButton.visualFocus ? "#83c7ff" : "transparent" }
            contentItem: Item {}
            onClicked: island.portClicked()
            ToolTip.delay: 650; ToolTip.timeout: 2500
            ToolTip.visible: hovered; ToolTip.text: "Harbor · processes by memory"
        }
        // A native vector signal route, sharing the scene clock and world space.
        Shape {
            anchors.fill: parent
            ShapePath {
                strokeColor: "#b5e7c48b"; strokeWidth: 1.6; fillColor: "transparent"
                startX: 475; startY: 155
                PathCubic { x: 355; y: 460; control1X: 670; control1Y: 245; control2X: 260; control2Y: 345 }
                PathCubic { x: 865; y: 355; control1X: 510; control1Y: 530; control2X: 730; control2Y: 320 }
                PathCubic { x: 1190; y: 540; control1X: 950; control1Y: 445; control2X: 1090; control2Y: 435 }
                PathCubic { x: 700; y: 740; control1X: 1450; control1Y: 760; control2X: 980; control2Y: 820 }
            }
        }
        Repeater {
            objectName: "signalLights"
            model: 36
            Rectangle {
                required property int index
                property var curves: [
                    [475,155,670,245,260,345,355,460],
                    [355,460,510,530,730,320,865,355],
                    [865,355,950,445,1090,435,1190,540],
                    [1190,540,1450,760,980,820,700,740]]
                property real progress: ((index / 36 + island.sceneTime * .003) % 1) * 4
                property var curve: curves[Math.floor(progress)]
                property real t: progress % 1
                function coordinate(offset) {
                    return Math.pow(1-t,3)*curve[offset] + 3*Math.pow(1-t,2)*t*curve[offset+2]
                         + 3*(1-t)*t*t*curve[offset+4] + t*t*t*curve[offset+6]
                }
                x: coordinate(0)-2; y: coordinate(1)-2
                width: 4; height: 4; radius: 2; color: "#fff0d4"
                Image { anchors.centerIn: parent; width: 19; height: 19; source: "qrc:/assets/glow.svg" }
            }
        }
        Repeater {
            model: [{x:475,y:155},{x:355,y:460},{x:865,y:355},{x:1190,y:540},{x:1392,y:180}]
            Item {
                required property var modelData
                x: modelData.x; y: modelData.y
                Image { anchors.centerIn: parent; width: 42; height: 42; source: "qrc:/assets/glow.svg"; opacity: .7 }
                Rectangle { anchors.centerIn: parent; width: 15; height: 15; radius: 8; color: "#f8ecd4"; border.width: 3; border.color: "#d5ad70" }
            }
        }

    }
    Repeater {
        model: [ {tag:cpuTag,x:475,y:155}, {tag:storageTag,x:355,y:460},
                 {tag:processTag,x:865,y:355}, {tag:alertsTag,x:1392,y:180},
                 {tag:networkTag,x:1190,y:540} ]
        Shape {
            required property var modelData
            anchors.fill: parent
            visible: island.width > 650
            ShapePath {
                strokeColor: "#cce3be83"; strokeWidth: 1; fillColor: "transparent"
                startX: modelData.tag.x + modelData.tag.width/2
                startY: modelData.tag.y + modelData.tag.height/2
                PathLine { x: world.x + modelData.x*world.scale; y: world.y + modelData.y*world.scale }
            }
        }
    }
    SceneTag {
        id: cpuTag
        objectName: "cpuSceneTag"
        x: Math.max(12, island.width*.43-width/2); y: 28
        iconName: "cpu"; samples: island.width > 600 ? island.cpuHistory : []
        text: "CPU"; value: island.cpuLoad < 0 ? "—" : island.cpuLoad.toFixed(0)+"%"
        active: volcanoButton.hovered; onClicked: island.volcanoClicked()
    }
    SceneTag {
        id: storageTag
        objectName: "storageSceneTag"
        x: island.width > 650 ? island.width*.1 : 14; y: island.height*.33
        iconName: "storage"; text: "Storage"
        value: island.storageRatio < 0 ? "—" : (island.storageRatio*100).toFixed(0)+"%"
        onClicked: island.storageClicked()
    }
    SceneTag {
        id: processTag
        objectName: "ramSceneTag"
        x: island.width > 650 ? island.width*.53 : island.width-width-14; y: island.height*(island.width > 650 ? .30 : .47)
        iconName: "activity"; text: "Processes"
        value: island.processCount < 0 ? "—" : island.processCount.toString()
        active: portButton.hovered; onClicked: island.portClicked()
        ToolTip.visible: hovered
        ToolTip.text: "Processes by memory · RAM " + (island.ramRatio < 0 ? "unavailable" : (island.ramRatio*100).toFixed(0)+"%")
    }
    SceneTag {
        id: alertsTag
        objectName: "alertsSceneTag"
        x: island.width-width-14; y: island.width < 600 ? 102 : 48
        iconName: "alerts"; text: "Alerts"
        value: island.alertCount ? island.alertCount + " alerts" : "All clear"
        onClicked: island.alertsClicked()
    }
    SceneTag {
        id: networkTag
        objectName: "networkSceneTag"
        x: island.width-width-14; y: island.width > 650 ? island.height*.63 : island.height-height-28
        height: 74
        iconName: "network"; text: "Network"
        value: island.networkValue + "\n" + island.networkOutValue
        samples: island.width > 600 ? island.receiveHistory : []; maximum: 0
        onClicked: island.networkClicked()
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
        property color accent: "#ceb47d"
        property string value: ""
        property bool active: false
        hoverEnabled: true
        property string iconName: "cpu"
        property var samples: []
        property real maximum: 100
        width: Math.max(136, tagContent.implicitWidth+26); height: 56
        Accessible.name: text + " " + value
        background: Rectangle {
            radius: 6; color: tag.hovered || tag.active ? "#f0102338" : "#ed081827"
            border.color: tag.hovered || tag.active || tag.activeFocus ? tag.accent : "#b89a65"
        }
        contentItem: Row {
            id: tagContent
            spacing: 12
            IconImage { source: "qrc:/assets/icons/"+tag.iconName+".svg"; sourceSize: Qt.size(48,48); width: 24; height: 24; color: tag.accent; anchors.verticalCenter: parent.verticalCenter }
            Column {
                anchors.verticalCenter: parent.verticalCenter; spacing: 3
                Text { text: tag.text; color: "#f1ede5"; font.pixelSize: 10 }
                Text { visible: tag.value.length > 0; text: tag.value; color: "#f1ede5"; font.pixelSize: 15; font.weight: Font.Medium }
            }
            HistoryPlot { visible: tag.samples.length > 0; width: 48; height: 26; values: tag.samples; color: tag.accent; maximum: tag.maximum; anchors.verticalCenter: parent.verticalCenter }
        }
        leftPadding: 11; rightPadding: 11
    }
}
