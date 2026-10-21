import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
ToolButton {
    id: control
    Theme { id: theme }
    property string iconName: "settings"
    property bool selected: false
    property bool light: false
    property color accent: theme.brass
    implicitWidth: 42; implicitHeight: 42
    hoverEnabled: true
    Accessible.name: text
    background: Rectangle {
        radius: 5
        color: control.down ? (control.light ? theme.line : theme.darkLine) : control.selected ? theme.navy : control.hovered ? (control.light ? theme.surface : theme.navy) : "transparent"
        border.width: 1
        border.color: control.activeFocus ? control.accent : control.selected ? theme.darkLine : "transparent"
        Rectangle { visible: control.selected; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: 2; height: 22; radius: 1; color: control.accent }
    }
    contentItem: Item {
        IconImage {
            anchors.centerIn: parent; width: 21; height: 21
            source: "qrc:/assets/icons/" + control.iconName + ".svg"
            sourceSize: Qt.size(42,42)
            color: control.selected ? theme.brass : control.light ? theme.navy : theme.ivory
            opacity: control.enabled ? (control.selected || control.hovered ? 1 : 0.75) : 0.3
        }
    }
    ToolTip.visible: hovered
    ToolTip.text: text
    ToolTip.delay: 500
}
