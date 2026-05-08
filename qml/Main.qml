import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 480
    height: 800
    visible: true
    title: "Kalah"

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: welcomeComp
    }

    Component { id: welcomeComp;    WelcomeScreen    {} }
    Component { id: nameComp;       NameScreen       {} }
    Component { id: genderComp;     GenderScreen     {} }
    Component { id: diffComp;       DifficultyScreen {} }
    Component { id: gameComp;       GameScreen       {} }

    Connections {
        target: game
        function onAppStateChanged() {
            switch (game.appState) {
            case 0: stack.replace(welcomeComp); break   // Welcome
            case 1: stack.push(nameComp);       break   // EnterName
            case 2: stack.push(genderComp);     break   // SelectGender
            case 3: stack.push(diffComp);       break   // SelectDifficulty
            case 4: stack.replace(gameComp);    break   // Playing
            }
        }
    }
}
