import QtQuick
import QtQuick.Controls

// A player's store (kalah) — the larger pit on either side of the board.
Rectangle {
    id: store

    property int stones: 0
    property string label: ""

    width: 110; height: 220
    radius: 55
    color: "#a07a4c"
    border.color: "#5a3a1a"
    border.width: 2

    Column {
        anchors.centerIn: parent
        spacing: 4
        Label {
            text: store.label
            font.pixelSize: 14
            color: "#2a1a08"
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: store.stones
            font.pixelSize: 36
            font.bold: true
            color: "#2a1a08"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
