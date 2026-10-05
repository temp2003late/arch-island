import QtQuick
import QtQuick.Controls
ToolButton {
    id: control
    property string iconName: "settings"
    property bool selected: false
    property color accent: "#3ba8ff"
    implicitWidth: 42; implicitHeight: 42
    hoverEnabled: true
    Accessible.name: text
    background: Rectangle {
        radius: 11
        color: control.down ? "#213d57" : control.selected ? "#162c43" : control.hovered ? "#132639" : "transparent"
        border.width: 1
        border.color: control.activeFocus ? control.accent : control.selected ? "#284866" : "transparent"
        Rectangle { visible: control.selected; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: 2; height: 22; radius: 1; color: control.accent }
    }
    contentItem: Item {
        Image {
            anchors.centerIn: parent; width: 21; height: 21
            source: "qrc:/assets/icons/" + control.iconName + ".svg"
            sourceSize: Qt.size(42,42)
            opacity: control.enabled ? (control.selected || control.hovered ? 1 : 0.75) : 0.3
        }
    }
    ToolTip.visible: hovered
    ToolTip.text: text
    ToolTip.delay: 500
}
