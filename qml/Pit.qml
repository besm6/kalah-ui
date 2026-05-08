import QtQuick
import QtQuick.Controls

// A single pit on a player's row. Tap to sow.
Rectangle {
    id: pit

    property int  pitIndex: -1
    property int  stones: 0
    property int  owner: 0          // 0 = bottom row, 1 = top row
    property bool playable: false   // highlight when it belongs to current player

    signal tapped(int index)

    width: 90; height: 90
    radius: width / 2
    color: playable ? "#d8b48a" : "#b8946a"
    border.color: "#5a3a1a"
    border.width: 2

    // Stone count label
    Label {
        anchors.centerIn: parent
        text: pit.stones
        font.pixelSize: 28
        font.bold: true
        color: "#2a1a08"
    }

    // Touch / mouse input. TapHandler works for both.
    TapHandler {
        enabled: pit.playable && pit.stones > 0
        onTapped: pit.tapped(pit.pitIndex)
    }

    // Subtle press feedback.
    Behavior on color { ColorAnimation { duration: 120 } }
}
