import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    Rectangle {
        anchors.fill: parent
        color: "#2a1e0e"
    }

    ColumnLayout {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 32
        }
        spacing: 16

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 32
            Layout.bottomMargin: 16
            text: "Select Difficulty"
            color: "#d4a84b"
            font.pixelSize: 32
            font.bold: true
        }

        Repeater {
            model: [
                { title: "Юноша",    sub: "Novice · great for beginners",        value: 1 },
                { title: "Кандидат", sub: "Candidate · a fair challenge",         value: 2 },
                { title: "Участник", sub: "Participant · for experienced players", value: 3 },
                { title: "Эфенди",   sub: "Master · maximum difficulty",          value: 4 }
            ]

            delegate: Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 10
                color: "#1a0f05"
                border.color: "#5a4030"
                border.width: 2

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.title
                        color: "#d4c8b0"
                        font.pixelSize: 22
                        font.bold: true
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.sub
                        color: "#907060"
                        font.pixelSize: 14
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onPressed: parent.border.color = "#d4a84b"
                    onReleased: parent.border.color = "#5a4030"
                    onClicked: game.selectLevel(modelData.value)
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
