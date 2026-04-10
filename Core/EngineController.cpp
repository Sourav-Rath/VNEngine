#include "EngineController.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>


EngineController::EngineController(QObject *parent)
    : QObject(parent)
{
    loadFromJson(":/VNEngine/Data/story.json");
}

void EngineController::loadFromJson(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open JSON:" << path;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
    {
        qDebug() << "Invalid JSON";
        return;
    }

    QJsonArray jsonNodes = doc.object()["nodes"].toArray();

    nodes.clear();

    for (auto value : jsonNodes)
    {
        QJsonObject obj = value.toObject();

        Node node;
        int id = obj["id"].toInt();

        node.text = obj["text"].toString();

        if (obj.contains("next"))
            node.nextNodeId = obj["next"].toInt();

        if (obj.contains("choices")) {
            QJsonArray arr = obj["choices"].toArray();

            for (const QJsonValue& cVal : arr) {
                QJsonObject cObj = cVal.toObject();

                Choice c;
                c.text = cObj["text"].toString();
                c.nextNodeId = cObj["next"].toInt();

                node.choices.append(c);
            }
        }

        nodes[id] = node;
    }

    currentNodeId = 0;

    qDebug() << "Nodes loaded:" << nodes.size();

    emit dialogueChanged();
}

QString EngineController::getCurrentDialogue() const
{
    if (!nodes.contains(currentNodeId))
        return "INVALID NODE";

    return nodes[currentNodeId].text;
}

QList<Choice> EngineController::getChoices() const
{
    if (!nodes.contains(currentNodeId))
        return {};

    return nodes[currentNodeId].choices;
}

void EngineController::next()
{
    if (!nodes.contains(currentNodeId))
        return;

    const Node& node = nodes[currentNodeId];

    if (!node.choices.isEmpty())
        return;

    if (node.nextNodeId != -1 && nodes.contains(node.nextNodeId)) {
        currentNodeId = node.nextNodeId;
        emit dialogueChanged();
    }
}

void EngineController::chooseOption(int index)
{
    if (!nodes.contains(currentNodeId))
        return;

    const Node& node = nodes[currentNodeId];

    if (index < 0 || index >= node.choices.size())
        return;

    int nextId = node.choices[index].nextNodeId;

    if (!nodes.contains(nextId))
        return;

    currentNodeId = nextId;
    emit dialogueChanged();
}