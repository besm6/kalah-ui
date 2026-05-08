#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "gamecontroller.h"

// Entry point of the application. Creates the Qt application object, sets up
// GameController (the bridge between QML and the game engine), injects it into QML
// as the "game" property, and loads Main.qml. Exits with an error if the QML file
// cannot be found or parsed.
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    GameController game;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("game", &game);
    engine.load(QUrl("qrc:/qt/qml/Kalah/qml/Main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
