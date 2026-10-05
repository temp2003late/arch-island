import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import ArchIsland 1.0
ApplicationWindow {
    id: root
    objectName: "mainWindow"
    visible: true
    width: 1400; height: 880
    minimumWidth: 800; minimumHeight: 640
    title: "Arch Island"
    color: "#07111e"
    font.family: "Adwaita Sans"
    palette.window: "#0c1b2c"
    palette.windowText: "#dbe8f7"
    palette.base: "#0a1828"
    palette.text: "#dbe8f7"
    palette.button: "#142c42"
    palette.buttonText: "#dbe8f7"
    palette.highlight: "#237abd"
    palette.highlightedText: "#e7f4ff"
    palette.mid: "#2b455e"
    property bool animationsActive: !preferences.reducedMotion && visibility !== Window.Minimized && visible
    function bytes(value) {
        if (value < 0) return "Unavailable"
        return metricNumber(value) + " " + metricUnit(value)
    }
    function metricNumber(value) {
        if (value < 0) return "—"
        if (value >= 1073741824) return (value/1073741824).toFixed(1)
        if (value >= 1048576) return (value/1048576).toFixed(1)
        if (value >= 1024) return (value/1024).toFixed(1)
        return value.toFixed(0)
    }
    function metricUnit(value) {
        if (value < 0) return ""
        if (value >= 1073741824) return "GiB"
        if (value >= 1048576) return "MiB"
        if (value >= 1024) return "KiB"
        return "B"
    }
    function openProcesses(byMemory) {
        monitor.sortByMemory = byMemory
        monitor.processesOpen = true
        processesDrawer.open()
    }
    function openSettings() { settingsDialog.open() }
    Rectangle {
        id: header
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 64
        gradient: Gradient { GradientStop { position: 0; color: "#101e30" } GradientStop { position: 1; color: "#091524" } }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#27374b" }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 13
            spacing: 13
            Image { source: "qrc:/assets/arch-island.svg"; sourceSize: Qt.size(76,76); Layout.preferredWidth: 33; Layout.preferredHeight: 37 }
            Label { text: "Arch Island"; color: "#eaf1ff"; font.pixelSize: 21; font.weight: Font.DemiBold }
            Label { visible: root.width > 1150; text: "SYSTEM MONITOR"; color: "#53728e"; font.pixelSize: 9; font.letterSpacing: 2; leftPadding: 10 }
            Item { Layout.fillWidth: true }
            Rectangle { implicitWidth: 7; implicitHeight: 7; radius: 4; color: monitor.cpu >= 0 ? "#39d7a5" : "#d5aa71" }
            Label { text: monitor.cpu >= 0 ? "Live" : "Sampling"; color: "#ccdcea"; font.pixelSize: 12 }
            Label { visible: root.width > 960; text: "·  " + (monitor.activeInterface || "No network"); color: "#6f8ba6"; font.pixelSize: 11; Layout.maximumWidth: 180; elide: Text.ElideRight }
            Rectangle { implicitWidth: 1; implicitHeight: 22; color: "#293c50"; Layout.leftMargin: 10; Layout.rightMargin: 5 }
            IconButton { objectName: "settingsButton"; text: "Settings"; iconName: "settings"; onClicked: root.openSettings() }
            IconButton { text: "Minimize"; iconName: "minimize"; onClicked: root.showMinimized() }
            IconButton { text: "Close Arch Island"; iconName: "close"; onClicked: root.close() }
        }
    }
    Item {
        id: body
        anchors.top: header.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: footer.top
        Rectangle {
            id: rail
            width: root.width < 1000 ? 54 : 64; height: parent.height
            color: "#0b1828"
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: "#1d3044" }
            Column {
                anchors.top: parent.top; anchors.topMargin: 18; anchors.horizontalCenter: parent.horizontalCenter; spacing: 12
                IconButton {
                    text: "Island overview"; iconName: "home"; selected: !monitor.processesOpen
                    onClicked: { processesDrawer.close(); settingsDialog.close() }
                }
                IconButton { text: "CPU processes"; iconName: "activity"; selected: monitor.processesOpen && !monitor.sortByMemory; onClicked: root.openProcesses(false) }
                IconButton { text: "Memory processes"; iconName: "memory"; selected: monitor.processesOpen && monitor.sortByMemory; onClicked: root.openProcesses(true) }
                IconButton { text: "Network interface"; iconName: "network"; onClicked: root.openSettings() }
            }
            Column {
                anchors.bottom: parent.bottom; anchors.bottomMargin: 16; anchors.horizontalCenter: parent.horizontalCenter; spacing: 9
                IconButton {
                    text: monitor.night ? "Switch to daytime" : "Switch to nighttime"
                    iconName: monitor.night ? "moon" : "sun"
                    onClicked: preferences.mode = monitor.night ? "day" : "night"
                }
            }
        }
        Island {
            id: islandScene
            objectName: "islandScene"
            anchors.left: rail.right; anchors.right: metricsPanel.left; anchors.top: parent.top; anchors.bottom: parent.bottom
            night: monitor.night; animate: root.animationsActive
            cpuLoad: monitor.cpu
            ramRatio: monitor.memoryTotal > 0 ? monitor.memoryUsed/monitor.memoryTotal : -1
            traffic: monitor.receiveRate >= 0 && monitor.sendRate >= 0 ? monitor.receiveRate+monitor.sendRate : -1
            onVolcanoClicked: root.openProcesses(false)
            onPortClicked: root.openProcesses(true)
            onNetworkClicked: root.openSettings()
        }
        Rectangle {
            id: metricsPanel
            anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.right: parent.right
            width: root.width < 1050 ? 222 : 266
            color: "#0a1626"
            Rectangle { width: 1; height: parent.height; color: "#273a50" }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 14; spacing: 8
                RowLayout {
                    Layout.fillWidth: true; Layout.bottomMargin: 4
                    Label { text: "SYSTEM RESOURCES"; color: "#6f8aa5"; font.pixelSize: 9; font.letterSpacing: 1.4; Layout.fillWidth: true }
                    Label { text: "1s"; color: "#55728e"; font.pixelSize: 9 }
                }
                MetricCard {
                    objectName: "cpuCard"
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "CPU"; iconName: "cpu"; accent: "#ff8250"
                    value: monitor.cpu < 0 ? "—" : monitor.cpu.toFixed(1)+"%"
                    detail: monitor.cpu < 0 ? "Waiting for a sample" : "All logical cores"
                    ratio: monitor.cpu < 0 ? -1 : monitor.cpu/100
                    history: monitor.cpuHistory
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "RAM"; iconName: "memory"; accent: "#369fff"
                    value: root.metricNumber(monitor.memoryUsed)
                    unit: root.metricUnit(monitor.memoryUsed)
                    detail: monitor.memoryTotal < 0 ? "Memory unavailable" : "of " + root.bytes(monitor.memoryTotal) + " physical memory"
                    ratio: monitor.memoryTotal > 0 ? monitor.memoryUsed/monitor.memoryTotal : -1
                    history: monitor.memoryHistory
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "RECEIVE"; iconName: "download"; accent: "#2bd4a7"
                    value: root.metricNumber(monitor.receiveRate)
                    unit: monitor.receiveRate < 0 ? "" : root.metricUnit(monitor.receiveRate)+"/s"
                    detail: monitor.receiveRate < 0 ? "Waiting for interface data" : "Incoming network traffic"
                    history: monitor.receiveHistory; maximum: 0
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "SEND"; iconName: "upload"; accent: "#a795ec"
                    value: root.metricNumber(monitor.sendRate)
                    unit: monitor.sendRate < 0 ? "" : root.metricUnit(monitor.sendRate)+"/s"
                    detail: monitor.sendRate < 0 ? "Waiting for interface data" : "Outgoing network traffic"
                    history: monitor.sendHistory; maximum: 0
                }
                Label {
                    Layout.fillWidth: true; Layout.topMargin: 4
                    text: preferences.reducedMotion ? "Reduced motion enabled" : "Local measurements · last 60 seconds"
                    color: "#536f8b"; font.pixelSize: 9; elide: Text.ElideRight
                }
            }
        }
    }
    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        height: root.width < 1000 ? 66 : 80
        gradient: Gradient { GradientStop { position: 0; color: "#0e1c2d" } GradientStop { position: 1; color: "#081320" } }
        Rectangle { width: parent.width; height: 1; color: "#2a3b50" }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 23; anchors.rightMargin: 22; anchors.topMargin: 12; anchors.bottomMargin: 10
            spacing: root.width < 1000 ? 15 : 25
            ColumnLayout {
                Layout.preferredWidth: root.width < 1000 ? 160 : 240; spacing: 5
                Label { text: "Your system, <font color='#36a8ff'>alive.</font>"; textFormat: Text.RichText; font.pixelSize: root.width < 1000 ? 15 : 19; font.weight: Font.Medium; color: "#b6cfe7" }
                Label { text: monitor.night ? "Moonlit shores. Real-time signals." : "Sunlit shores. Real-time signals."; color: "#526f8b"; font.pixelSize: 9; visible: root.width > 1000 }
            }
            Rectangle { implicitWidth: 1; Layout.fillHeight: true; color: "#25384c" }
            FooterChart { label: "CPU"; tint: "#ff8250"; samples: monitor.cpuHistory; ceiling: 100; Layout.fillWidth: true; Layout.fillHeight: true }
            FooterChart { label: "RAM"; tint: "#369fff"; samples: monitor.memoryHistory; ceiling: 100; Layout.fillWidth: true; Layout.fillHeight: true }
            FooterChart { label: "NETWORK"; tint: "#2bd4a7"; samples: monitor.receiveHistory; secondary: monitor.sendHistory; ceiling: 0; Layout.fillWidth: true; Layout.fillHeight: true }
            Image { visible: root.width > 1150; source: "qrc:/assets/icons/"+(monitor.night ? "moon" : "sun")+".svg"; sourceSize: Qt.size(48,48); Layout.preferredWidth: 24; Layout.preferredHeight: 24; Layout.leftMargin: 6 }
        }
    }
    Drawer {
        id: processesDrawer
        objectName: "processesDrawer"
        edge: Qt.RightEdge
        width: Math.min(root.width-32,650); height: root.height
        modal: true
        onClosed: monitor.processesOpen = false
        background: Rectangle { color: "#0b192a"; border.color: "#30465f" }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 25; spacing: 15
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 5
                    Label { text: "Processes"; font.pixelSize: 25; font.weight: Font.Medium; color: "#e3efff" }
                    Label { text: "A closer look at your island."; color: "#708daa"; font.pixelSize: 12 }
                }
                IconButton { text: "Close processes"; iconName: "close"; onClicked: processesDrawer.close() }
            }
            RowLayout {
                Layout.topMargin: 7; spacing: 9
                ActionButton { text: "CPU usage"; highlighted: !monitor.sortByMemory; onClicked: monitor.sortByMemory = false }
                ActionButton { text: "Memory"; highlighted: monitor.sortByMemory; onClicked: monitor.sortByMemory = true }
                Item { Layout.fillWidth: true }
                Label { text: "READ ONLY"; color: "#55738f"; font.pixelSize: 9; font.letterSpacing: 1 }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: infoLabel.implicitHeight+24
                radius: 10; color: "#112338"; border.color: "#233c55"
                Label {
                    id: infoLabel
                    anchors.fill: parent; anchors.margins: 12
                    text: "100% CPU = one fully occupied logical core.\nMemory is RSS; shared pages can belong to several processes."
                    color: "#98b4cf"; wrapMode: Text.WordWrap; font.pixelSize: 11
                }
            }
            Label { text: monitor.processStatus; color: "#65849f"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 10 }
            RowLayout {
                Layout.fillWidth: true; spacing: 12
                Label { text: "PID"; Layout.preferredWidth: 58; color: "#657f9c"; font.pixelSize: 10 }
                Label { text: "PROCESS"; Layout.fillWidth: true; color: "#657f9c"; font.pixelSize: 10 }
                Label { text: "CPU"; Layout.preferredWidth: 72; color: "#657f9c"; horizontalAlignment: Text.AlignRight; font.pixelSize: 10 }
                Label { text: "MEMORY"; Layout.preferredWidth: 90; color: "#657f9c"; horizontalAlignment: Text.AlignRight; font.pixelSize: 10 }
            }
            ListView {
                objectName: "processList"
                Layout.fillWidth: true; Layout.fillHeight: true
                model: monitor.processes; clip: true; spacing: 2
                ScrollBar.vertical: ScrollBar {}
                delegate: Rectangle {
                    required property int index
                    required property var processPid
                    required property string processName
                    required property real processCpu
                    required property real processMemory
                    width: ListView.view.width; height: 42; radius: 6
                    color: index%2 ? "#102338" : "#0b192a"
                    RowLayout {
                        anchors.fill: parent; spacing: 12
                        Label { text: processPid; Layout.preferredWidth: 58; color: "#6283a2"; font.pixelSize: 11 }
                        Label { text: processName; Layout.fillWidth: true; elide: Text.ElideRight; color: "#d9e6f5"; font.pixelSize: 12; textFormat: Text.PlainText }
                        Label { text: processCpu < 0 ? "—" : processCpu.toFixed(1)+"%"; Layout.preferredWidth: 72; horizontalAlignment: Text.AlignRight; color: "#f5a573"; font.pixelSize: 11 }
                        Label { text: root.bytes(processMemory); Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight; color: "#84b9ed"; font.pixelSize: 11 }
                    }
                }
            }
        }
    }
    Dialog {
        id: settingsDialog
        objectName: "settingsDialog"
        title: "Island settings"
        anchors.centerIn: parent
        modal: true
        width: Math.min(root.width-60,460)
        padding: 23
        background: Rectangle { color: "#0d1d30"; radius: 17; border.color: "#3b526a" }
        header: Label { text: "Island settings"; color: "#e7f0fc"; font.pixelSize: 23; font.weight: Font.Medium; leftPadding: 23; topPadding: 23; bottomPadding: 4 }
        contentItem: ColumnLayout {
            spacing: 17
            Label { text: "Make yourself at home."; color: "#748fab"; font.pixelSize: 12; Layout.bottomMargin: 7 }
            Label { text: "ATMOSPHERE"; color: "#7c9bb9"; font.pixelSize: 10; font.letterSpacing: 1.3 }
            SettingsCombo {
                objectName: "modeSelector"
                Layout.fillWidth: true
                model: ["Automatic · local time", "Daylight", "Moonlight"]
                currentIndex: preferences.mode === "day" ? 1 : preferences.mode === "night" ? 2 : 0
                onActivated: preferences.mode = ["auto","day","night"][currentIndex]
            }
            Label { text: "Night falls at 19:00. Sunrise is at 07:00.\nAutomatic mode follows your local clock."; color: "#7796b2"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            CheckBox {
                id: motionSwitch
                objectName: "reducedMotionSwitch"
                text: "Reduce decorative animation"
                checked: preferences.reducedMotion
                onToggled: preferences.reducedMotion = checked
                spacing: 10
                indicator: Rectangle {
                    width: 20; height: 20; y: (motionSwitch.height-height)/2; radius: 5
                    color: motionSwitch.checked ? "#237bb7" : "#102337"
                    border.color: motionSwitch.activeFocus ? "#82c9fb" : "#3a5973"
                    Text { anchors.centerIn: parent; text: motionSwitch.checked ? "✓" : ""; color: "#e0f5ff"; font.pixelSize: 14 }
                }
                contentItem: Text { text: motionSwitch.text; leftPadding: 30; color: "#c4d8eb"; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
            }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#213950" }
            Label { text: "NETWORK INTERFACE"; color: "#7c9bb9"; font.pixelSize: 10; font.letterSpacing: 1.3 }
            SettingsCombo {
                objectName: "interfaceSelector"
                Layout.fillWidth: true
                property var choices: {
                    let list = ["Automatic"]
                    for (let name of monitor.interfaces) list.push(name)
                    if (preferences.networkInterface && list.indexOf(preferences.networkInterface) < 0) list.push(preferences.networkInterface)
                    return list
                }
                model: choices
                currentIndex: preferences.networkInterface ? choices.indexOf(preferences.networkInterface) : 0
                onActivated: preferences.networkInterface = currentIndex === 0 ? "" : choices[currentIndex]
            }
            Label { text: "Tracks one interface at a time. Automatic prefers the default IPv4 route, then a physical device."; color: "#7796b2"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: "Saved on this computer. Always private."; color: "#53748f"; font.pixelSize: 10; Layout.topMargin: 5 }
        }
        footer: Item {
            implicitHeight: 65
            ActionButton { anchors.right: parent.right; anchors.rightMargin: 23; anchors.top: parent.top; anchors.topMargin: 4; text: "Done"; highlighted: true; onClicked: settingsDialog.close() }
        }
    }
    component SettingsCombo: ComboBox {
        id: combo
        implicitHeight: 42
        leftPadding: 13; rightPadding: 32
        background: Rectangle { radius: 9; color: "#12283e"; border.color: combo.activeFocus ? "#489ddc" : "#2c4861" }
        contentItem: Text { text: combo.displayText; color: "#d3e5f5"; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        indicator: Text { x: combo.width-width-13; y: (combo.height-height)/2-2; text: "⌄"; color: "#82afd1"; font.pixelSize: 20 }
    }
    component FooterChart: Item {
        property string label: ""
        property color tint: "#369fff"
        property var samples: []
        property var secondary: []
        property real ceiling: 100
        Label { text: parent.label; color: "#5d7995"; font.pixelSize: 8; font.letterSpacing: 1; anchors.top: parent.top }
        HistoryPlot { anchors.fill: parent; anchors.topMargin: 13; values: parent.samples; secondaryValues: parent.secondary; color: parent.tint; maximum: parent.ceiling }
    }
}
