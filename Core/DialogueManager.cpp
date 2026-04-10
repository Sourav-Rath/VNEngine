#include "DialogueManager.h"
#include <QDebug>

DialogueManager::DialogueManager(QObject *parent)
    : QObject(parent)
{
    m_state.clear(); // RESET STATE

    m_choiceModel.setChoices({});
    loadFromJson(":/VNEngine/Data/story.json");

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

    QList<Choice> processedChoices;

    for (Choice choice : node.choices)
    {
        evaluateChoice(choice);
        processedChoices.append(choice);
    }

    m_choiceModel.setChoices(processedChoices);

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

void DialogueManager::selectChoice(int index)
{
    if (!nodes.contains(currentNodeId))
        return;

    const Node& node = nodes[currentNodeId];

    if (index < 0 || index >= node.choices.size())
        return;

    Choice choice = node.choices[index];

    // Validate condition
    evaluateChoice(choice);
    if (!choice.isEnabled)
    {
        qDebug() << "Blocked choice due to unmet conditions";
        return;
    }

    // Apply flags
    for (auto it = choice.setFlags.begin(); it != choice.setFlags.end(); ++it)
    {
        qDebug() << "Setting flag:" << it.key() << "=" << it.value();
        setFlag(it.key(), it.value());
    }

    setCurrentNode(choice.nextNodeId);
}

void DialogueManager::setFlag(const QString& key, const QVariant& value)
{
    m_state[key] = value;
}

QVariant DialogueManager::getFlag(const QString& key) const
{
    return m_state.value(key, QVariant());
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

            // setFlags
            if (choiceObj.contains("setFlags")) {
                choice.setFlags = choiceObj["setFlags"].toObject().toVariantMap();
            }

            // conditions
            if (choiceObj.contains("conditions")) {
                choice.conditions = choiceObj["conditions"].toObject().toVariantMap();
            }

            node.choices.append(choice);
        }

        nodes[id] = node;
    }

    qDebug() << "Loaded nodes:" << nodes.size();
}

bool DialogueManager::evaluateChoice(Choice& choice)
{
    if (choice.conditions.isEmpty())
    {
        choice.isEnabled = true;
        return true;
    }

    QStringList requirements;
    bool valid = true;

    for (auto it = choice.conditions.begin(); it != choice.conditions.end(); ++it)
    {
        // Check if flag exists
        if (!m_state.contains(it.key()))
        {
            valid = false;

            requirements.append(
                QString("%1 required (NOT SET / %2)")
                    .arg(it.key())
                    .arg(it.value().toString())
                );
            continue;
        }

        QVariant current = m_state.value(it.key());

        // Strict comparison
        if (current != it.value())
        {
            valid = false;

            requirements.append(
                QString("%1 required (%2 / %3)")
                    .arg(it.key())
                    .arg(current.toString())
                    .arg(it.value().toString())
                );
        }
    }

    choice.isEnabled = valid;
    choice.requirementText = requirements.join(", ");

    return valid;
}