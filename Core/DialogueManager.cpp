#include "DialogueManager.h"
#include <QDebug>

DialogueManager::DialogueManager(QObject *parent)
    : QObject(parent)
{
    // Step 1 — Initialize model (prevents null issues in QML)
    m_choiceModel.setChoices({});

    // Step 2 — Load data
    loadFromJson(":/VNEngine/Data/story.json");

    // Step 3 — Set first node ONLY if data exists
    if (!nodes.isEmpty()) {
        setCurrentNode(0);
    }
}

QString DialogueManager::currentText() const
{
    if (!nodes.contains(currentNodeId))
        return "INVALID NODE";

    return nodes[currentNodeId].text;
}

ChoiceModel* DialogueManager::choicesModel()
{
    return &m_choiceModel;
}

void DialogueManager::setCurrentNode(int nodeId)
{
    if (!nodes.contains(nodeId)) {
        qDebug() << "Invalid node:" << nodeId;
        return;
    }

    currentNodeId = nodeId;

    const Node &node = nodes[currentNodeId];

    m_choiceModel.setChoices(node.choices);

    emit dialogueChanged();
    emit choicesChanged();
}

void DialogueManager::next()
{
    if (!nodes.contains(currentNodeId))
        return;

    int nextId = nodes[currentNodeId].nextNodeId;

    if (nextId != -1)
        setCurrentNode(nextId);
}

void DialogueManager::selectChoice(int nextNodeId)
{
    setCurrentNode(nextNodeId);
}

void DialogueManager::loadFromJson(const QString &path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open JSON:" << path;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray jsonNodes = root["nodes"].toArray();

    for (const QJsonValue &value : jsonNodes) {
        QJsonObject obj = value.toObject();

        Node node;
        int id = obj["id"].toInt();

        node.text = obj["text"].toString();
        node.nextNodeId = obj["nextNodeId"].toInt(obj["next"].toInt(-1));

        QJsonArray choicesArray = obj["choices"].toArray();

        for (const QJsonValue &choiceVal : choicesArray) {
            QJsonObject choiceObj = choiceVal.toObject();

            Choice choice;
            choice.text = choiceObj["text"].toString();
            choice.nextNodeId = choiceObj["nextNodeId"].toInt(choiceObj["next"].toInt());

            node.choices.append(choice);
        }

        nodes[id] = node;
    }

    qDebug() << "Loaded nodes:" << nodes.size();
}