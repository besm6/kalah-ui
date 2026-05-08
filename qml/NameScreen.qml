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
        spacing: 0

        Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 32
            text: "Enter Your Name"
            color: "#d4a84b"
            font.pixelSize: 32
            font.bold: true
        }

        Item { Layout.fillHeight: true }

        TextField {
            id: tf
            Layout.fillWidth: true
            Layout.preferredHeight: 72
            placeholderText: "Your name"
            maximumLength: 24
            font.pixelSize: 28
            color: "#f0e6d0"
            placeholderTextColor: "#806040"
            background: Rectangle {
                radius: 8
                color: "#1a0f05"
                border.color: tf.activeFocus ? "#d4a84b" : "#5a4030"
                border.width: 2
            }
            leftPadding: 16
            Component.onCompleted: forceActiveFocus()
            Keys.onReturnPressed: if (tf.text.trim().length > 0) game.submitName(tf.text.trim())
        }

        Item { Layout.fillHeight: true }

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            Layout.bottomMargin: 32
            text: "Continue"
            enabled: tf.text.trim().length > 0
            font.pixelSize: 22
            onClicked: game.submitName(tf.text.trim())

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: parent.enabled ? "#1a0f05" : "#806040"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 8
                color: parent.enabled ? "#d4a84b" : "#3a2a10"
                border.color: parent.enabled ? "#f0c860" : "#5a4030"
                border.width: 2
            }
        }
    }
}
