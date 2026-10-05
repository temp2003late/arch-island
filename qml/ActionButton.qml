import QtQuick
import QtQuick.Controls
Button {
    id: control
    implicitHeight: 38
    implicitWidth: Math.max(90, contentItem.implicitWidth + 30)
    hoverEnabled: true
    leftPadding: 15; rightPadding: 15
    background: Rectangle {
        radius: 9
        color: control.highlighted ? "#183b58" : control.hovered ? "#1b3045" : "#102135"
        border.color: control.activeFocus ? "#74c2ff" : control.highlighted ? "#3673a2" : "#273d53"
    }
    contentItem: Text {
        text: control.text; color: control.highlighted ? "#a6daff" : "#c7d7e8"
        font.pixelSize: 12; font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
    }
}
