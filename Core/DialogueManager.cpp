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

    // Validate again (backend safety)
    evaluateChoice(choice);
    if (!choice.isEnabled)
    {
        qDebug() << "Blocked choice";
        return;
    }

    // Apply flags
    for (auto it = choice.setFlags.begin(); it != choice.setFlags.end(); ++it)
    {
        QString key = it.key();
        QVariant value = it.value();

        QVariant current = m_state.value(key, 0);

        //  If simple value → SET
        if (!value.canConvert<QVariantMap>())
        {
            qDebug() << "SET:" << key << "=" << value;
            setFlag(key, value);
            continue;
        }

        QVariantMap opMap = value.toMap();

        for (auto op = opMap.begin(); op != opMap.end(); ++op)
        {
            QString operation = op.key();
            QVariant operand = op.value();

            double curr = current.toDouble();
            double val = operand.toDouble();

            if (operation == "set")
            {
                current = operand;
            }
            else if (operation == "add")
            {
                current = curr + val;
            }
            else if (operation == "sub")
            {
                current = curr - val;
            }
            else if (operation == "mul")
            {
                current = curr * val;
            }
            else if (operation == "div")
            {
                if (val != 0)
                    current = curr / val;
            }
            else
            {
                qDebug() << "Unknown operation:" << operation;
            }
        }

        qDebug() << "Updated:" << key << "=" << current;
        setFlag(key, current);
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
        QString key = it.key();

        if (!m_state.contains(key))
        {
            valid = false;
            requirements.append(QString("%1 NOT SET").arg(key));
            continue;
        }

        QVariant current = m_state.value(key);
        QVariantMap conditionMap = it.value().toMap();

        for (auto cond = conditionMap.begin(); cond != conditionMap.end(); ++cond)
        {
            QString op = cond.key();
            QVariant expected = cond.value();

            bool result = true;

            // 🔥 OPERATOR LOGIC
            if (op == "eq")
                result = (current == expected);

            else if (op == "neq")
                result = (current != expected);

            else if (op == "gt")
                result = (current.toDouble() > expected.toDouble());

            else if (op == "gte")
                result = (current.toDouble() >= expected.toDouble());

            else if (op == "lt")
                result = (current.toDouble() < expected.toDouble());

            else if (op == "lte")
                result = (current.toDouble() <= expected.toDouble());

            else
            {
                result = false;
                requirements.append(QString("Unknown op: %1").arg(op));
            }

            if (!result)
            {
                valid = false;

                requirements.append(
                    QString("%1 %2 %3 (current: %4)")
                        .arg(key)
                        .arg(op)
                        .arg(expected.toString())
                        .arg(current.toString())
                    );
            }
        }
    }

    choice.isEnabled = valid;
    choice.requirementText = requirements.join(", ");

    return valid;
}

//Save Function
void DialogueManager::saveGame()
{
    QJsonObject saveObj;

    // Save current node
    saveObj["currentNodeId"] = currentNodeId;

    // Save state
    QJsonObject stateObj;
    for (auto it = m_state.begin(); it != m_state.end(); ++it)
    {
        stateObj[it.key()] = QJsonValue::fromVariant(it.value());
    }

    saveObj["state"] = stateObj;

    QJsonDocument doc(saveObj);

    QFile file("save.json");
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to save game";
        return;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Game saved!";
}

//Load Function
void DialogueManager::loadGame()
{
    QFile file("save.json");

    if (!file.exists())
    {
        qDebug() << "No save file found";
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open save file";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject saveObj = doc.object();

    // Load node
    currentNodeId = saveObj["currentNodeId"].toInt();

    // Load state
    m_state.clear();
    QJsonObject stateObj = saveObj["state"].toObject();

    for (auto it = stateObj.begin(); it != stateObj.end(); ++it)
    {
        m_state[it.key()] = it.value().toVariant();
    }

    // Apply node
    setCurrentNode(currentNodeId);

    qDebug() << "Game loaded!";
}

// Restart System
void DialogueManager::restartGame()
{
    qDebug() << "Restarting game...";

    //  Reset state
    m_state.clear();

    // Reload JSON fresh
    nodes.clear();
    loadFromJson(":/VNEngine/Data/story.json");

    // Reset to first node
    if (!nodes.isEmpty())
    {
        currentNodeId = 0;
        setCurrentNode(0);
    }
}