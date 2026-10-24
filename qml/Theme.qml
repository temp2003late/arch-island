import QtQuick

// Arthelm brand tokens shared by the native interface.
QtObject {
    property bool night: false
    // All interface colors follow the island's single day/night transition.
    property real darkness: night ? 1 : 0
    function blend(dayColor: color, nightColor: color): color {
        return Qt.rgba(dayColor.r + (nightColor.r-dayColor.r)*darkness,
                       dayColor.g + (nightColor.g-dayColor.g)*darkness,
                       dayColor.b + (nightColor.b-dayColor.b)*darkness,
                       dayColor.a + (nightColor.a-dayColor.a)*darkness)
    }
    readonly property color panel: blend(ivory, "#0d1c2b")
    readonly property color card: blend("#f3efe8", "#142638")
    readonly property color text: blend(navy, "#e1e7eb")
    readonly property color navy: "#102338"
    readonly property color deep: "#081827"
    readonly property color ivory: "#f1ede5"
    readonly property color surface: blend("#e8e3d9", "#21374a")
    readonly property color brass: "#ceb47d"
    readonly property color brassMuted: "#b89a65"
    readonly property color secondary: blend("#526171", "#a6b5c2")
    readonly property color line: blend("#d0ccc3", "#2d4255")
    readonly property color darkLine: "#314253"
    readonly property color onDark: "#b9c2c9"
    readonly property color cpu: blend("#b48759", "#cda374")
    readonly property color memory: blend("#738fa5", "#91b3cc")
    readonly property color network: blend("#68948b", "#82b7aa")
    readonly property color send: blend("#9e91ac", "#b5a3cb")
}
