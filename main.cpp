#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


#include "Core/DialogueManager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    DialogueManager* dialogueManager = new DialogueManager(&engine);
    engine.rootContext()->setContextProperty("dialogueManager", dialogueManager);

    engine.load(QUrl("qrc:/VNEngine/Main.qml"));

    return app.exec();
}