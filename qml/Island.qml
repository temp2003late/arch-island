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
        // Fill the scene; overlay labels and the boat remain inside narrow viewports.
        scale: Math.max(island.width/1600, island.height/1000)
        Image {
            id: dayArt
            objectName: "dayArtwork"
            anchors.fill: parent
            source: "qrc:/assets/art/island-day.png"
            smooth: true; mipmap: true
        }
        Image {
            id: nightArt
            objectName: "nightArtwork"
            anchors.fill: parent
            source: "qrc:/assets/art/island-night.png"
            opacity: island.night ? 1 : 0
            smooth: true; mipmap: true
            Behavior on opacity { NumberAnimation { duration: island.animate ? 1000 : 0; easing.type: Easing.InOutSine } }
        }
        // Only these small textured overlays animate; the detailed environment is static.
        Image {
            x: 469; y: 30; width: 160; height: 160
            source: "qrc:/assets/glow.svg"; sourceSize: Qt.size(240,240)
            opacity: island.cpuLoad < 0 ? 0 : 0.16+island.heat*0.72
            Behavior on opacity { NumberAnimation { duration: island.animate ? 800 : 0 } }
        }
        Repeater {
            model: 6
            Image {
                id: smoke
                required property int index
                property real phase: 0.15 + index*0.11
                x: 504+phase*48+index*3; y: 85-phase*95
                width: 48+phase*70; height: width
                source: "qrc:/assets/smoke.svg"; sourceSize: Qt.size(128,128)
                visible: island.cpuLoad >= 0
                opacity: (1-phase)*(0.10+island.heat*0.5)
                SequentialAnimation on phase {
                    running: island.animate && island.cpuLoad >= 0; loops: Animation.Infinite
                    PauseAnimation { duration: smoke.index*430 }
                    NumberAnimation { from: 0; to: 1; duration: 4300; easing.type: Easing.OutSine }
                }
            }
        }
        Repeater {
            model: 7
            Rectangle {
                id: ember
                required property int index
                property real phase: 0
                visible: island.cpuLoad > 30
                x: 546+Math.sin(index*4.3)*phase*32
                y: 111-phase*(35+island.heat*80)
                width: 1.5+index%2; height: width; radius: width
                color: "#ffb553"; opacity: (1-phase)*island.heat
                SequentialAnimation on phase {
                    running: island.animate && island.cpuLoad > 30; loops: Animation.Infinite
                    PauseAnimation { duration: ember.index*220 }
                    NumberAnimation { from: 0; to: 1; duration: 1600+ember.index*180 }
                }
            }
        }
        Image {
            x: 1204; y: 157; width: 500; height: 146
            source: "qrc:/assets/beam.svg"; sourceSize: Qt.size(750,219)
            visible: island.night; opacity: 0.7
            transformOrigin: Item.Left
            SequentialAnimation on rotation {
                running: island.animate && island.night; loops: Animation.Infinite
                NumberAnimation { from: -16; to: 14; duration: 7200; easing.type: Easing.InOutSine }
                NumberAnimation { from: 14; to: -16; duration: 7200; easing.type: Easing.InOutSine }
            }
        }
        // Additional foreground freight is tied to the actual used-memory ratio.
        Repeater {
            model: island.ramRatio < 0 ? 0 : Math.ceil(island.ramRatio*14)
            Image {
                required property int index
                x: 1025+(index%5)*27-Math.floor(index/5)*8
                y: 628+(index%5)*3-Math.floor(index/5)*22
                width: 31; height: 29
                source: "qrc:/assets/cargo.svg"; sourceSize: Qt.size(62,58)
                opacity: island.night ? 0.83 : 1
            }
        }
        Item {
            id: boat
            property real voyage: 0
            x: Math.max(209,(25-(island.width-world.width*world.scale)/2)/world.scale)+voyage*90
            y: 645+voyage*20
            width: 220; height: 220
            Image {
                x: -60; y: 177; width: 200; height: 54
                source: "qrc:/assets/wake.svg"; sourceSize: Qt.size(400,108)
                opacity: island.traffic > 0 ? 0.7 : 0.2
            }
            Image {
                id: sailingBoat
                objectName: "sailboatArtwork"
                anchors.fill: parent; source: "qrc:/assets/art/sailboat.png"
                smooth: true; mipmap: true
            }
            SequentialAnimation on voyage {
                running: island.animate && island.traffic > 0; loops: Animation.Infinite
                NumberAnimation { from: 0; to: 1; duration: Math.max(6000,27000/(1+Math.log(1+Math.max(0,island.traffic)/4096))); easing.type: Easing.InOutSine }
                NumberAnimation { from: 1; to: 0; duration: Math.max(6000,27000/(1+Math.log(1+Math.max(0,island.traffic)/4096))); easing.type: Easing.InOutSine }
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
