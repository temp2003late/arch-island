import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ArchIsland 1.0
Rectangle {
    id: card
    property string caption: "CPU"
    property string value: "—"
    property string unit: ""
    property string detail: ""
    property string iconName: "cpu"
    property color accent: "#ff8656"
    property real ratio: -1
    property var history: []
    property real maximum: 100
    property bool compact: height < 142
    implicitHeight: 156
    radius: 16
    border.color: "#223348"
    gradient: Gradient {
        GradientStop { position: 0; color: "#142337" }
        GradientStop { position: 1; color: "#0c192a" }
    }
    Rectangle { x: 16; y: 0; width: parent.width-32; height: 1; color: "#23384e"; opacity: 0.6 }
    RowLayout {
        id: summary
        x: 13; y: card.compact ? 11 : 17
        width: parent.width-26; spacing: 11
        Item {
            Layout.preferredWidth: card.compact ? 42 : 54
            Layout.preferredHeight: Layout.preferredWidth
            GaugeArc { anchors.fill: parent; value: card.ratio; color: card.accent }
            Image {
                anchors.centerIn: parent; width: card.compact ? 19 : 23; height: width
                source: "qrc:/assets/icons/" + card.iconName + ".svg"
                sourceSize: Qt.size(46,46)
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 5
            Label { text: card.caption; color: "#9bafc7"; font.pixelSize: 11; font.letterSpacing: 0.8; Layout.fillWidth: true; elide: Text.ElideRight }
            RowLayout {
                Layout.fillWidth: true; spacing: 5
                Label {
                    text: card.value; color: "#ecf3fd"
                    font.pixelSize: card.compact ? 23 : 28; font.weight: Font.Medium
                    Layout.minimumWidth: 0; elide: Text.ElideRight
                }
                Label { text: card.unit; color: "#a7bbd0"; font.pixelSize: 12; Layout.alignment: Qt.AlignBottom; bottomPadding: 4 }
                Item { Layout.fillWidth: true }
            }
        }
    }
    Label {
        x: 17; anchors.top: summary.bottom; anchors.topMargin: card.compact ? 5 : 9
        width: parent.width-34
        text: card.detail; color: "#7792ac"; font.pixelSize: 10
        elide: Text.ElideRight
    }
    HistoryPlot {
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        anchors.leftMargin: 13; anchors.rightMargin: 13; anchors.bottomMargin: 10
        height: Math.max(21, card.height-summary.height-summary.y-37)
        values: card.history; color: card.accent; maximum: card.maximum
    }
}
