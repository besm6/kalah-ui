import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    function pitCount(i) { return game.pits[i] }

    Rectangle {
        anchors.fill: parent
        color: "#3b2a14"
    }

    ColumnLayout {
        anchors {
            fill: parent
            margins: 16
        }
        spacing: 8

        // Score bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                text: game.userName
                color: "#d4a84b"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }

            Text {
                text: root.pitCount(6) + " — " + root.pitCount(13)
                color: "#c8b89a"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            Text {
                text: "Jinn"
                color: "#a08060"
                font.pixelSize: 18
                font.bold: true
                horizontalAlignment: Text.AlignRight
                Layout.fillWidth: true
            }
        }

        // Status label
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: {
                if (game.gameOver) {
                    if (game.winner === -1) return "Tie game!"
                    return (game.winner === 0 ? game.userName : "Jinn") + " wins!"
                }
                if (game.aiThinking) return "Jinn is thinking…"
                return "Your turn"
            }
            color: game.gameOver ? "#d4a84b" : (game.aiThinking ? "#a08060" : "#c8b89a")
            font.pixelSize: 20

            SequentialAnimation on opacity {
                running: game.aiThinking
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 600 }
                NumberAnimation { to: 1.0; duration: 600 }
            }
        }

        // Board row: JINN store | pits | USER store
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // JINN store (left)
            Store {
                stones: root.pitCount(13)
                label: "Jinn"
            }

            // Pit grid: top row = JINN (12→7), bottom row = USER (0→5)
            Grid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 6
                rows: 2
                rowSpacing: 8
                columnSpacing: 8

                // Top row: JINN pits right-to-left (12, 11, 10, 9, 8, 7)
                Repeater {
                    model: 6
                    Pit {
                        property int boardIndex: 12 - index
                        pitIndex: boardIndex
                        owner: 1
                        stones: root.pitCount(boardIndex)
                        playable: false
                    }
                }

                // Bottom row: USER pits left-to-right (0..5)
                Repeater {
                    model: 6
                    Pit {
                        pitIndex: index
                        owner: 0
                        stones: root.pitCount(index)
                        playable: !game.gameOver && !game.aiThinking && game.currentPlayer === 0
                        onTapped: (i) => game.sow(i)
                    }
                }
            }

            // USER store (right)
            Store {
                stones: root.pitCount(6)
                label: game.userName
            }
        }

        // New Game button
        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 56
            Layout.preferredWidth: parent.width * 0.6
            text: "New Game"
            font.pixelSize: 20
            onClicked: game.newGame()

            contentItem: Text {
                text: parent.text
                font: parent.font
                color: "#1a0f05"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 8
                color: "#d4a84b"
                border.color: "#f0c860"
                border.width: 2
            }
        }
    }

    Connections {
        target: game
        function onIllegalMove(pitIndex) {
            // future: shake animation on the tapped pit
        }
    }
}
