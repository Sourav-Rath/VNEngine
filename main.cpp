#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>


#include "Core/DialogueManager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    DialogueManager dialogueManager;

    engine.rootContext()->setContextProperty("dialogueManager", &dialogueManager);

    engine.loadFromModule("VNEngine", "Main");

    return app.exec();
}