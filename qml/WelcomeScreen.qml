import QtQuick
import QtQuick.Controls

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a0f05" }
            GradientStop { position: 1.0; color: "#3b2a14" }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 24
        width: parent.width * 0.8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Kalah"
            color: "#d4a84b"
            font.pixelSize: 64
            font.bold: true
            font.family: "Georgia"
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "A classic strategy game\nof skill and planning"
            color: "#c8b89a"
            font.pixelSize: 22
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.4
        }
    }

    Text {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 64
        }
        text: "Tap anywhere to begin"
        color: "#a09070"
        font.pixelSize: 18

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { to: 1.06; duration: 900; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1.0;  duration: 900; easing.type: Easing.InOutSine }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: game.proceedFromWelcome()
    }
}
