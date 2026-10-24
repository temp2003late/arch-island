import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import ArchIsland 1.0
Rectangle {
    id: card
    property bool night: false
    property real darkness: night ? 1 : 0
    Theme { id: theme; night: card.night; darkness: card.darkness }
    property string caption: "CPU"
    property string value: "—"
    property string unit: ""
    property string detail: ""
    property string iconName: "cpu"
    property color accent: theme.cpu
    property real ratio: -1
    property var history: []
    property real maximum: 100
    property bool bar: false
    readonly property bool compact: height < 90
    implicitHeight: 124
    radius: 9
    color: theme.card
    border.color: theme.line
    HoverHandler { id: hover }
    ToolTip.visible: hover.hovered && card.compact
    ToolTip.text: card.detail
    ToolTip.delay: 500
    RowLayout {
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: parent.top; anchors.bottom: card.compact ? parent.bottom : detailLabel.top
        anchors.margins: card.compact ? 8 : 14; anchors.bottomMargin: card.compact ? 8 : 5
        spacing: 12
        Rectangle {
            Layout.preferredWidth: card.width < 240 ? 32 : 42
            Layout.preferredHeight: width
            radius: width/2; color: theme.surface
            IconImage {
                anchors.centerIn: parent; width: 23; height: 23
                source: "qrc:/assets/icons/" + card.iconName + ".svg"
                sourceSize: Qt.size(46,46); color: theme.text
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 5
            Label { text: card.caption; color: theme.text; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
            RowLayout {
                Layout.fillWidth: true; spacing: 4
                Label {
                    text: card.value; color: theme.text
                    font.pixelSize: card.width < 240 ? 22 : 27; font.weight: Font.Medium
                    Layout.minimumWidth: 0; elide: Text.ElideRight
                }
                Label { text: card.unit; color: theme.secondary; font.pixelSize: 10; Layout.alignment: Qt.AlignBottom; bottomPadding: 4 }
                Item {
                    Layout.fillWidth: true; Layout.minimumWidth: 15; Layout.preferredHeight: 32
                    HistoryPlot { anchors.fill: parent; visible: !card.bar; values: card.history; color: card.accent; maximum: card.maximum }
                    Rectangle {
                        visible: card.bar; anchors.verticalCenter: parent.verticalCenter
                        width: parent.width; height: 7; radius: 4; color: theme.surface
                        Rectangle { width: parent.width*Math.max(0,Math.min(1,card.ratio)); height: parent.height; radius: 4; color: card.accent }
                    }
                }
            }
        }
    }
    Label {
        id: detailLabel
        visible: !card.compact
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        anchors.leftMargin: 16; anchors.rightMargin: 12; anchors.bottomMargin: 12
        text: card.detail; color: theme.secondary; font.pixelSize: 10
        elide: Text.ElideRight
    }
}
