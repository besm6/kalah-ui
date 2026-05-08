import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1024; height: 600
    visible: true
    title: "Mancala"
    color: "#3b2a14"

    // Convenience: read pit counts from the C++ model.
    function pitCount(i) { return game.pits[i] }

    Column {
        anchors.centerIn: parent
        spacing: 24

        // Status bar
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: game.gameOver
                  ? (game.winner === -1 ? "Tie game"
                                        : "Player " + (game.winner + 1) + " wins")
                  : "Player " + (game.currentPlayer + 1) + "'s turn"
            color: "white"
            font.pixelSize: 22
        }

        // Board: P1 store | grid of pits | P0 store
        RowLayout {
            spacing: 16

            // Player 1's store (left)
            Store {
                stones: root.pitCount(13)
                label:  "Player 2"
            }

            // Two rows of 6 pits.
            Grid {
                columns: 6
                rows: 2
                rowSpacing: 16
                columnSpacing: 16
                layoutDirection: Qt.LeftToRight

                // Top row = player 1's pits, displayed right-to-left
                // so index 7 sits on the right above index 5.
                Repeater {
                    model: 6
                    Pit {
                        property int boardIndex: 12 - index   // 12,11,10,9,8,7
                        pitIndex: boardIndex
                        owner:    1
                        stones:   root.pitCount(boardIndex)
                        playable: !game.gameOver && game.currentPlayer === 1
                        onTapped: (i) => game.sow(i)
                    }
                }

                // Bottom row = player 0's pits, left-to-right (0..5)
                Repeater {
                    model: 6
                    Pit {
                        pitIndex: index
                        owner:    0
                        stones:   root.pitCount(index)
                        playable: !game.gameOver && game.currentPlayer === 0
                        onTapped: (i) => game.sow(i)
                    }
                }
            }

            // Player 0's store (right)
            Store {
                stones: root.pitCount(6)
                label:  "Player 1"
            }
        }

        // Reset button
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "New game"
            onClicked: game.reset()
        }
    }

    // Optional: react to engine signals for animation hooks, sounds, etc.
    Connections {
        target: game
        function onMoveCompleted(lastIndex, extraTurn, capture) {
            // Hook point: trigger stone-flying animations or a capture flash here.
        }
        function onIllegalMove(pitIndex) {
            // Hook point: shake the tapped pit, play a buzz sound, etc.
        }
    }
}
