import QtQuick
import QtQuick.Controls
Button {
    id: control
    Theme { id: theme }
    implicitHeight: 38
    implicitWidth: Math.max(90, contentItem.implicitWidth + 30)
    hoverEnabled: true
    leftPadding: 15; rightPadding: 15
    background: Rectangle {
        radius: 5
        color: control.highlighted ? theme.brass : control.hovered ? theme.darkLine : theme.navy
        border.color: control.activeFocus ? theme.brass : control.highlighted ? theme.brassMuted : theme.darkLine
    }
    contentItem: Text {
        text: control.text; color: control.highlighted ? theme.deep : theme.ivory
        font.pixelSize: 12; font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
    }
}
