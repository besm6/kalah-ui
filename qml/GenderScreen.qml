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
            text: "Select Your Gender"
            color: "#d4a84b"
            font.pixelSize: 32
            font.bold: true
        }

        Repeater {
            model: [
                { label: "Male",              value: 1 },
                { label: "Female",            value: 2 },
                { label: "Prefer not to say", value: 0 }
            ]

            delegate: Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                radius: 10
                color: "#1a0f05"
                border.color: "#5a4030"
                border.width: 2

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: "#d4c8b0"
                    font.pixelSize: 24
                }

                MouseArea {
                    anchors.fill: parent
                    onPressed: parent.border.color = "#d4a84b"
                    onReleased: parent.border.color = "#5a4030"
                    onClicked: game.selectGender(modelData.value)
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
