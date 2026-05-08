#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "mancalagame.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    MancalaGame game;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("game", &game);
    engine.load(QUrl("qrc:/Mancala/qml/Main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
