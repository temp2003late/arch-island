import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts
import QtQuick.Window
import ArchIsland 1.0
ApplicationWindow {
    id: root
    Theme { id: theme }
    objectName: "mainWindow"
    visible: true
    width: 1600; height: 900
    minimumWidth: 800; minimumHeight: 640
    title: "Arthelm Beacon"
    color: theme.deep
    font.family: interfaceFont.name
    FontLoader { id: interfaceFont; source: "qrc:/assets/fonts/inter.ttf" }
    FontLoader { id: displayFont; source: "qrc:/assets/fonts/fraunces.ttf" }
    palette.window: theme.deep
    palette.windowText: theme.ivory
    palette.base: theme.deep
    palette.text: theme.ivory
    palette.button: theme.navy
    palette.buttonText: theme.ivory
    palette.highlight: theme.brass
    palette.highlightedText: theme.deep
    palette.mid: theme.darkLine
    property bool animationsActive: !preferences.reducedMotion && !preferences.systemReducedMotion && visibility !== Window.Minimized && visible
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
        height: 88
        color: theme.ivory
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: theme.line }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 13
            spacing: 13
            Image { source: "qrc:/assets/brand/arthelm.webp"; sourceSize: Qt.size(104,104); Layout.preferredWidth: 52; Layout.preferredHeight: 52 }
            Rectangle { implicitWidth: 1; implicitHeight: 38; color: theme.line }
            Label { text: "Arthelm Beacon"; color: theme.navy; font.family: displayFont.name; font.pixelSize: root.width < 1000 ? 24 : 32 }
            Label { visible: root.width > 1300; text: "LINUX SYSTEM MONITOR"; color: theme.secondary; font.pixelSize: 9; font.letterSpacing: 2; leftPadding: 12 }
            Rectangle {
                visible: root.width > 1100; implicitWidth: 130; implicitHeight: 26; radius: 13
                color: "transparent"; border.color: theme.brass
                Label { anchors.centerIn: parent; text: "●  In Development"; color: theme.brassMuted; font.pixelSize: 10 }
            }
            Item { Layout.fillWidth: true }
            Rectangle { implicitWidth: 7; implicitHeight: 7; radius: 4; color: monitor.cpu >= 0 ? theme.network : theme.brassMuted }
            Label { text: monitor.cpu >= 0 ? "Live" : "Sampling"; color: theme.navy; font.pixelSize: 12 }
            Label { visible: root.width > 960; text: "·  " + (monitor.activeInterface || "No network"); color: theme.secondary; font.pixelSize: 11; Layout.maximumWidth: 180; elide: Text.ElideRight }
            Rectangle { implicitWidth: 1; implicitHeight: 22; color: theme.darkLine; Layout.leftMargin: 10; Layout.rightMargin: 5 }
            IconButton {
                light: true; text: monitor.night ? "Switch to daytime" : "Switch to nighttime"
                iconName: monitor.night ? "moon" : "sun"
                onClicked: preferences.mode = monitor.night ? "day" : "night"
            }
            IconButton { light: true; objectName: "settingsButton"; text: "Settings"; iconName: "settings"; onClicked: root.openSettings() }
            IconButton { light: true; text: "Minimize"; iconName: "minimize"; onClicked: root.showMinimized() }
            IconButton { light: true; text: root.visibility === Window.Maximized ? "Restore window" : "Maximize"; iconName: "maximize"; onClicked: root.visibility === Window.Maximized ? root.showNormal() : root.showMaximized() }
            IconButton { light: true; text: "Close Arthelm Beacon"; iconName: "close"; onClicked: root.close() }
        }
    }
    Item {
        id: body
        anchors.top: header.bottom; anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: footer.top
        Rectangle {
            id: rail
            width: root.width < 1150 ? 58 : 176; height: parent.height
            color: theme.deep
            Column {
                anchors.top: parent.top; anchors.topMargin: 18; anchors.horizontalCenter: parent.horizontalCenter; spacing: 9
                NavButton { objectName: "overviewNav"; text: "Overview"; iconName: "home"; selected: !monitor.processesOpen && !settingsDialog.visible && !networkDialog.visible && !storageDialog.visible && !alertsDialog.visible; onClicked: { processesDrawer.close(); networkDialog.close(); settingsDialog.close(); storageDialog.close(); alertsDialog.close() } }
                NavButton { objectName: "processesNav"; text: "Processes"; iconName: "activity"; selected: monitor.processesOpen; onClicked: root.openProcesses(false) }
                NavButton { objectName: "networkNav"; text: "Network"; iconName: "network"; selected: networkDialog.visible; onClicked: networkDialog.open() }
                NavButton { objectName: "storageNav"; text: "Storage"; iconName: "storage"; selected: storageDialog.visible; onClicked: storageDialog.open() }
                NavButton { objectName: "alertsNav"; text: "Alerts"; iconName: "alerts"; selected: alertsDialog.visible; onClicked: alertsDialog.open() }
            }
            Column {
                visible: rail.width > 100
                anchors.bottom: parent.bottom; anchors.bottomMargin: 24; anchors.left: parent.left; anchors.leftMargin: 22; spacing: 12
                Rectangle { width: 130; height: 1; color: theme.darkLine }
                Image { source: "qrc:/assets/brand/arthelm.webp"; width: 44; height: 44; opacity: .75 }
                Label { text: "Technology.\nAt your helm."; color: theme.onDark; font.pixelSize: 12; lineHeight: 1.3 }
            }
        }
        Island {
            id: islandScene
            objectName: "islandScene"
            anchors.left: rail.right; anchors.right: metricsPanel.left; anchors.top: parent.top; anchors.bottom: parent.bottom
            night: monitor.night; animate: root.animationsActive
            cpuHistory: monitor.cpuHistory; memoryHistory: monitor.memoryHistory
            networkValue: "↓ " + root.bytes(monitor.receiveRate) + (monitor.receiveRate >= 0 ? "/s" : "")
            storageRatio: monitor.storageTotal > 0 ? monitor.storageUsed / monitor.storageTotal : -1
            processCount: monitor.processCount
            alertCount: monitor.alerts.length
            networkOutValue: "↑ " + root.bytes(monitor.sendRate) + (monitor.sendRate >= 0 ? "/s" : "")
            receiveHistory: monitor.receiveHistory
            onStorageClicked: storageDialog.open()
            onAlertsClicked: alertsDialog.open()
            cpuLoad: monitor.cpu
            ramRatio: monitor.memoryTotal > 0 ? monitor.memoryUsed/monitor.memoryTotal : -1
            traffic: monitor.receiveRate >= 0 && monitor.sendRate >= 0 ? monitor.receiveRate+monitor.sendRate : -1
            onVolcanoClicked: root.openProcesses(false)
            onPortClicked: root.openProcesses(true)
            onNetworkClicked: networkDialog.open()
        }
        Rectangle {
            id: metricsPanel
            anchors.top: parent.top; anchors.bottom: parent.bottom; anchors.right: parent.right
            width: root.width < 1050 ? 238 : 310
            color: theme.ivory
            Rectangle { width: 1; height: parent.height; color: theme.line }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: root.width < 1050 ? 12 : 20; spacing: 10
                RowLayout {
                    Layout.fillWidth: true; Layout.bottomMargin: 4
                    Label { text: "System Metrics"; color: theme.navy; font.family: displayFont.name; font.pixelSize: 22; Layout.fillWidth: true }
                    Label { text: "1s"; color: theme.secondary; font.pixelSize: 9 }
                }
                MetricCard {
                    objectName: "cpuCard"
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "CPU"; iconName: "cpu"; accent: theme.cpu
                    value: monitor.cpu < 0 ? "—" : monitor.cpu.toFixed(1)+"%"
                    detail: monitor.cpu < 0 ? "Waiting for a sample" : "All logical cores"
                    ratio: monitor.cpu < 0 ? -1 : monitor.cpu/100
                    history: monitor.cpuHistory
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "RAM"; iconName: "memory"; accent: theme.memory
                    value: root.metricNumber(monitor.memoryUsed)
                    unit: root.metricUnit(monitor.memoryUsed)
                    detail: monitor.memoryTotal < 0 ? "Memory unavailable" : "of " + root.bytes(monitor.memoryTotal) + " physical memory"
                    ratio: monitor.memoryTotal > 0 ? monitor.memoryUsed/monitor.memoryTotal : -1
                    history: monitor.memoryHistory
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "Network In"; iconName: "download"; accent: theme.network
                    value: root.metricNumber(monitor.receiveRate)
                    unit: monitor.receiveRate < 0 ? "" : root.metricUnit(monitor.receiveRate)+"/s"
                    detail: monitor.receiveRate < 0 ? "Waiting for interface data" : "Incoming network traffic"
                    history: monitor.receiveHistory; maximum: 0
                }
                MetricCard {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "Network Out"; iconName: "upload"; accent: theme.send
                    value: root.metricNumber(monitor.sendRate)
                    unit: monitor.sendRate < 0 ? "" : root.metricUnit(monitor.sendRate)+"/s"
                    detail: monitor.sendRate < 0 ? "Waiting for interface data" : "Outgoing network traffic"
                    history: monitor.sendHistory; maximum: 0
                }
                MetricCard {
                    objectName: "storageCard"
                    Layout.fillWidth: true; Layout.fillHeight: true
                    caption: "Storage Used"; iconName: "storage"; accent: theme.brassMuted; bar: true
                    value: monitor.storageTotal > 0 ? (monitor.storageUsed/monitor.storageTotal*100).toFixed(0)+"%" : "—"
                    ratio: monitor.storageTotal > 0 ? monitor.storageUsed/monitor.storageTotal : -1
                    detail: monitor.storageTotal > 0 ? "/  ·  " + root.bytes(monitor.storageTotal) + "  ·  " + monitor.storageFormat : "Root filesystem unavailable"
                }
                Label {
                    Layout.fillWidth: true; Layout.topMargin: 4
                    text: preferences.reducedMotion || preferences.systemReducedMotion ? "Reduced motion enabled" : "Local measurements · last 60 samples"
                    color: theme.secondary; font.pixelSize: 9; elide: Text.ElideRight
                }
            }
        }
    }
    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
        height: root.width < 1000 ? 66 : 80
        color: theme.ivory
        Rectangle { width: parent.width; height: 1; color: theme.line }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 25; anchors.rightMargin: 25
            spacing: root.width < 1000 ? 16 : 30
            ColumnLayout {
                Layout.preferredWidth: root.width < 1000 ? 205 : 350; spacing: 3
                Label { text: "Your system, charted."; font.family: displayFont.name; font.pixelSize: root.width < 1000 ? 19 : 26; color: theme.navy }
                Label { text: "Live signals. Clear direction. Full control."; color: theme.secondary; font.pixelSize: 11; visible: root.width > 1000 }
            }
            Rectangle { implicitWidth: 1; implicitHeight: 38; color: theme.line }
            FooterInfo { iconName: "computer"; label: monitor.hostName; value: monitor.kernelVersion; Layout.fillWidth: true }
            Rectangle { implicitWidth: 1; implicitHeight: 38; color: theme.line }
            FooterInfo {
                iconName: "uptime"; label: "Uptime"; Layout.fillWidth: true
                value: monitor.uptime < 0 ? "Unavailable" : Math.floor(monitor.uptime/86400) + "d " + Math.floor(monitor.uptime%86400/3600) + "h " + Math.floor(monitor.uptime%3600/60) + "m"
            }
            Rectangle { visible: root.width > 1200; implicitWidth: 1; implicitHeight: 38; color: theme.line }
            FooterInfo { visible: root.width > 1200; iconName: "network"; label: "Interface"; value: monitor.activeInterface || "Unavailable"; Layout.fillWidth: true }
            Item {
                visible: root.width > 1400; Layout.preferredWidth: 170; Layout.preferredHeight: 44
                HistoryPlot { anchors.fill: parent; values: monitor.cpuHistory; color: theme.memory; maximum: 100 }
                Label { anchors.right: parent.right; text: "Signals in view."; color: theme.navy; font.pixelSize: 10 }
            }
        }
    }
    component FooterInfo: RowLayout {
        property string iconName
        property string label
        property string value
        spacing: 14
        IconImage { source: "qrc:/assets/icons/"+parent.iconName+".svg"; sourceSize: Qt.size(48,48); Layout.preferredWidth: 24; Layout.preferredHeight: 24; color: theme.navy }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 5
            Label { text: parent.parent.label; color: theme.navy; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight; textFormat: Text.PlainText }
            Label { text: parent.parent.value; color: theme.secondary; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight; textFormat: Text.PlainText }
        }
    }
    component NavButton: Button {
        id: nav
        property string iconName
        property bool selected: false
        width: rail.width-14; height: 56
        hoverEnabled: true
        Accessible.name: text
        background: Rectangle {
            radius: 6; color: nav.selected || nav.hovered ? theme.navy : "transparent"
            border.color: nav.activeFocus ? theme.brass : "transparent"
            Rectangle { visible: nav.selected; width: 3; height: parent.height-4; y: 2; radius: 2; color: theme.brass }
        }
        contentItem: RowLayout {
            spacing: 16
            IconImage { Layout.leftMargin: rail.width > 100 ? 12 : 4; source: "qrc:/assets/icons/"+nav.iconName+".svg"; sourceSize: Qt.size(48,48); Layout.preferredWidth: 24; Layout.preferredHeight: 24; color: nav.selected ? theme.brass : theme.onDark }
            Label { visible: rail.width > 100; text: nav.text; color: nav.selected ? theme.brass : theme.onDark; font.pixelSize: 13; Layout.fillWidth: true }
        }
        ToolTip.visible: hovered && rail.width < 100
        ToolTip.text: text
    }
    Dialog {
        id: networkDialog
        objectName: "networkDialog"
        anchors.centerIn: parent; width: Math.min(root.width-60, 560); modal: true
        title: "Network · " + (monitor.activeInterface || "Unavailable")
        standardButtons: Dialog.Close
        background: Rectangle { color: theme.navy; radius: 8; border.color: theme.darkLine }
        contentItem: ColumnLayout {
            spacing: 16
            Label { text: "Incoming  ↓ " + root.bytes(monitor.receiveRate) + (monitor.receiveRate >= 0 ? "/s" : ""); color: theme.ivory }
            HistoryPlot { Layout.fillWidth: true; Layout.preferredHeight: 80; values: monitor.receiveHistory; color: theme.network; maximum: 0 }
            Label { text: "Outgoing  ↑ " + root.bytes(monitor.sendRate) + (monitor.sendRate >= 0 ? "/s" : ""); color: theme.ivory }
            HistoryPlot { Layout.fillWidth: true; Layout.preferredHeight: 80; values: monitor.sendHistory; color: theme.send; maximum: 0 }
            Label { text: "Last 60 samples · about one per second · one interface"; color: theme.onDark }
            ActionButton { text: "Choose interface…"; onClicked: { networkDialog.close(); root.openSettings() } }
        }
    }
    Dialog {
        id: storageDialog
        objectName: "storageDialog"
        anchors.centerIn: parent; width: Math.min(root.width-60,460); modal: true
        title: "Root filesystem"; standardButtons: Dialog.Close
        background: Rectangle { color: theme.navy; radius: 8; border.color: theme.darkLine }
        contentItem: Label {
            text: monitor.storageTotal > 0 ? root.bytes(monitor.storageUsed) + " used of " + root.bytes(monitor.storageTotal) + "\n" + root.bytes(monitor.storageTotal-monitor.storageUsed) + " free\n\nMount: / · " + monitor.storageFormat + "\nUpdated every 10 seconds. Usage is not disk health." : "Root filesystem data unavailable."
            wrapMode: Text.WordWrap; color: theme.ivory; lineHeight: 1.5
        }
    }
    Dialog {
        id: alertsDialog
        objectName: "alertsDialog"
        anchors.centerIn: parent; width: Math.min(root.width-60,460); modal: true
        title: "Monitoring alerts"; standardButtons: Dialog.Close
        background: Rectangle { color: theme.navy; radius: 8; border.color: theme.darkLine }
        contentItem: Label {
            text: (monitor.alerts.length ? monitor.alerts.join("\n") : "No monitoring alerts.") + "\n\nChecks cover unavailable CPU, memory, network and storage data, and root filesystem usage of 90% or more."
            wrapMode: Text.WordWrap; color: theme.ivory; lineHeight: 1.5
        }
    }
    Drawer {
        id: processesDrawer
        objectName: "processesDrawer"
        edge: Qt.RightEdge
        width: Math.min(root.width-32,650); height: root.height
        modal: true
        onClosed: monitor.processesOpen = false
        background: Rectangle { color: theme.deep; border.color: theme.darkLine }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 25; spacing: 15
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 5
                    Label { text: "Processes"; font.family: displayFont.name; font.pixelSize: 28; font.weight: Font.Medium; color: theme.ivory }
                    Label { text: "A closer look at your island."; color: theme.onDark; font.pixelSize: 12 }
                }
                IconButton { text: "Close processes"; iconName: "close"; onClicked: processesDrawer.close() }
            }
            RowLayout {
                Layout.topMargin: 7; spacing: 9
                ActionButton { text: "CPU usage"; highlighted: !monitor.sortByMemory; onClicked: monitor.sortByMemory = false }
                ActionButton { text: "Memory"; highlighted: monitor.sortByMemory; onClicked: monitor.sortByMemory = true }
                Item { Layout.fillWidth: true }
                Label { text: "READ ONLY"; color: theme.onDark; font.pixelSize: 9; font.letterSpacing: 1 }
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: infoLabel.implicitHeight+24
                radius: 6; color: theme.navy; border.color: theme.darkLine
                Label {
                    id: infoLabel
                    anchors.fill: parent; anchors.margins: 12
                    text: "100% CPU = one fully occupied logical core.\nMemory is RSS; shared pages can belong to several processes."
                    color: theme.onDark; wrapMode: Text.WordWrap; font.pixelSize: 11
                }
            }
            Label { text: monitor.processStatus; color: theme.onDark; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 10 }
            RowLayout {
                Layout.fillWidth: true; spacing: 12
                Label { text: "PID"; Layout.preferredWidth: 58; color: theme.onDark; font.pixelSize: 10 }
                Label { text: "PROCESS"; Layout.fillWidth: true; color: theme.onDark; font.pixelSize: 10 }
                Label { text: "CPU"; Layout.preferredWidth: 72; color: theme.onDark; horizontalAlignment: Text.AlignRight; font.pixelSize: 10 }
                Label { text: "MEMORY"; Layout.preferredWidth: 90; color: theme.onDark; horizontalAlignment: Text.AlignRight; font.pixelSize: 10 }
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
                    color: index%2 ? theme.navy : theme.deep
                    RowLayout {
                        anchors.fill: parent; spacing: 12
                        Label { text: processPid; Layout.preferredWidth: 58; color: theme.onDark; font.pixelSize: 11 }
                        Label { text: processName; Layout.fillWidth: true; elide: Text.ElideRight; color: theme.ivory; font.pixelSize: 12; textFormat: Text.PlainText }
                        Label { text: processCpu < 0 ? "—" : processCpu.toFixed(1)+"%"; Layout.preferredWidth: 72; horizontalAlignment: Text.AlignRight; color: theme.onDark; font.pixelSize: 11 }
                        Label { text: root.bytes(processMemory); Layout.preferredWidth: 90; horizontalAlignment: Text.AlignRight; color: theme.onDark; font.pixelSize: 11 }
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
        background: Rectangle { color: theme.navy; radius: 8; border.color: theme.onDark }
        header: Label { text: "Island settings"; color: theme.ivory; font.family: displayFont.name; font.pixelSize: 26; font.weight: Font.Medium; leftPadding: 23; topPadding: 23; bottomPadding: 4 }
        contentItem: ColumnLayout {
            spacing: 17
            Label { text: "Make yourself at home."; color: theme.onDark; font.pixelSize: 12; Layout.bottomMargin: 7 }
            Label { text: "ATMOSPHERE"; color: theme.onDark; font.pixelSize: 10; font.letterSpacing: 1.3 }
            SettingsCombo {
                objectName: "modeSelector"
                Layout.fillWidth: true
                model: ["Automatic · local time", "Daylight", "Moonlight"]
                currentIndex: preferences.mode === "day" ? 1 : preferences.mode === "night" ? 2 : 0
                onActivated: preferences.mode = ["auto","day","night"][currentIndex]
            }
            Label { text: "Night falls at 19:00. Sunrise is at 07:00.\nAutomatic mode follows your local clock."; color: theme.onDark; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            CheckBox {
                id: motionSwitch
                objectName: "reducedMotionSwitch"
                text: "Reduce decorative animation"
                checked: preferences.reducedMotion
                onToggled: preferences.reducedMotion = checked
                spacing: 10
                indicator: Rectangle {
                    width: 20; height: 20; y: (motionSwitch.height-height)/2; radius: 5
                    color: motionSwitch.checked ? theme.brassMuted : theme.deep
                    border.color: motionSwitch.activeFocus ? theme.brass : theme.onDark
                    Text { anchors.centerIn: parent; text: motionSwitch.checked ? "✓" : ""; color: theme.deep; font.pixelSize: 14 }
                }
                contentItem: Text { text: motionSwitch.text; leftPadding: 30; color: theme.ivory; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter }
            }
            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: theme.darkLine }
            Label { text: "NETWORK INTERFACE"; color: theme.onDark; font.pixelSize: 10; font.letterSpacing: 1.3 }
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
            Label { text: "Tracks one interface at a time. Automatic follows the lowest-metric IPv4 default route. Equal metrics keep the current interface."; color: theme.onDark; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: "Saved on this computer. Always private."; color: theme.onDark; font.pixelSize: 10; Layout.topMargin: 5 }
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
        background: Rectangle { radius: 5; color: theme.navy; border.color: combo.activeFocus ? theme.brass : theme.darkLine }
        contentItem: Text { text: combo.displayText; color: theme.ivory; font.pixelSize: 12; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight }
        indicator: Text { x: combo.width-width-13; y: (combo.height-height)/2-2; text: "⌄"; color: theme.onDark; font.pixelSize: 20 }
    }
}
