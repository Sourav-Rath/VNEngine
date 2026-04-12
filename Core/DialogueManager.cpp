#include "DialogueManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

// ================= CONSTRUCTOR =================

DialogueManager::DialogueManager(QObject *parent)
    : QObject(parent)
{
    m_state.clear();
    m_choiceModel.setChoices({});

    loadFromJson(":/VNEngine/Data/story.json");

    qDebug() << "Exists:"
             << QFile::exists(":/VNEngine/Data/story.json");

    if (!nodes.isEmpty()) {
        setCurrentNode(nodes.firstKey());
    }
}

// ================= BASIC =================

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

// ================= NODE =================

void DialogueManager::setCurrentNode(int nodeId)
{
    if (!nodes.contains(nodeId)) {
        qDebug() << "Invalid node:" << nodeId;
        return;
    }

    currentNodeId = nodeId;

    const Node& node = nodes[currentNodeId];

    QList<Choice> processed;

    for (Choice choice : node.choices)
    {
        evaluateChoice(choice);
        processed.append(choice);
    }

    m_choiceModel.setChoices(processed);

    emit dialogueChanged();
    emit choicesChanged();

    if (!node.events.isEmpty())
        executeEvents(node.events);
}

// ================= CHOICE =================

void DialogueManager::evaluateChoice(Choice& choice)
{
    choice.isEnabled = true;
    choice.requirement = "";

    for (auto it = choice.conditions.begin(); it != choice.conditions.end(); ++it)
    {
        QVariant current = m_state.value(it.key(), QVariant());
        QVariant required = it.value();

        // 🔥 INT → >= comparison
        if (current.canConvert<int>() && required.canConvert<int>())
        {
            if (current.toInt() < required.toInt())
            {
                choice.isEnabled = false;
                choice.requirement = it.key() + ": " + required.toString();
                return;
            }
        }
        // 🔥 BOOL / OTHER → equality
        else
        {
            if (current != required)
            {
                choice.isEnabled = false;
                choice.requirement = it.key() + ": " + required.toString();
                return;
            }
        }
    }
}

void DialogueManager::selectChoice(int index)
{
    const QList<Choice>& choices = m_choiceModel.getChoices();

    if (index < 0 || index >= choices.size())
    {
        m_inputLocked = false;
        emit inputLockedChanged();
        return;
    }

    Choice choice = choices[index];

    if (!choice.isEnabled)
    {
        m_inputLocked = false;
        emit inputLockedChanged();
        return;
    }


    // APPLY FLAGS
    for (auto it = choice.setFlags.begin(); it != choice.setFlags.end(); ++it)
    {
        setFlag(it.key(), it.value());
    }

    if (!choice.events.isEmpty())
    {
        executeEvents(choice.events);

        QVariantMap transition;
        transition["type"] = "transition";
        transition["nextNode"] = choice.nextNodeId;

        eventQueue.enqueue(transition);
    }
    else
    {
        setCurrentNode(choice.nextNodeId);

        // UNLOCK because no event system will handle it
        m_inputLocked = false;
        emit inputLockedChanged();

    }
}

// ================= EVENTS =================

void DialogueManager::executeEvents(const QList<QVariantMap>& events)
{

    for (const auto& ev : events)
        eventQueue.enqueue(ev);

    m_inputLocked = true;
    emit inputLockedChanged();

    QTimer::singleShot(0, this, [this]() {
        processNextEvent();
    });
}

void DialogueManager::processNextEvent()
{
    if (eventQueue.isEmpty())
    {
        // UNLOCK INPUT HERE
        m_inputLocked = false;
        emit inputLockedChanged();
        return;
    }

    QVariantMap ev = eventQueue.dequeue();
    QString type = ev["type"].toString();

    if (type == "print")
    {
        emit eventPrint(ev["message"].toString());
        QTimer::singleShot(0, this, [this]() {
    processNextEvent();
});
    }
    else if (type == "log")
    {
        emit eventLog(ev["message"].toString());
        QTimer::singleShot(0, this, [this]() {
        processNextEvent();
});
    }
    else if (type == "sound")
    {
        emit eventSound(ev["file"].toString());
        QTimer::singleShot(0, this, [this]() {
        processNextEvent();
});
    }
    else if (type == "delay")
    {
        int time = ev["time"].toInt();

        QTimer::singleShot(time, this, [this]() {
            QTimer::singleShot(0, this, [this]() {
            processNextEvent();
});
        });
    }
    else if (type == "transition")
    {
        int nextNode = ev["nextNode"].toInt();

        setCurrentNode(nextNode);

        QTimer::singleShot(0, this, [this]() {
        processNextEvent();
});
    }
}

// ================= STATE =================

void DialogueManager::setFlag(const QString& key, const QVariant& value)
{
    // BOOL → overwrite
    if (value.typeId() == QMetaType::Bool)
    {
        m_state[key] = value;
    }
    // INT → stack
    else if (value.canConvert<int>())
    {
        int current = m_state.value(key, 0).toInt();
        m_state[key] = current + value.toInt();
    }
    else
    {
        m_state[key] = value;
    }

    qDebug() << "FLAG SET:" << key << "=" << m_state[key];
}

QVariant DialogueManager::getFlag(const QString& key) const
{
    return m_state.value(key, QVariant());
}

// ================= JSON =================

void DialogueManager::loadFromJson(const QString& path)
{
    nodes.clear();

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "FAILED TO OPEN JSON:" << path;
        return;
    }

    qDebug() << "SUCCESSFULLY OPENED JSON:" << path;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON Parse Error:" << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qDebug() << "JSON root is not object!";
        return;
    }

    QJsonObject root = doc.object();

    if (!root.contains("nodes")) {
        qDebug() << "JSON missing 'nodes' key!";
        return;
    }

    QJsonArray array = root["nodes"].toArray();

    for (const QJsonValue& val : array) {
        QJsonObject obj = val.toObject();

        Node node;
        node.id = obj["id"].toInt();
        node.text = obj["text"].toString();

        node.nextNodeId = obj.contains("next")
                              ? obj["next"].toInt()
                              : -1;

        // CHOICES
        if (obj.contains("choices")) {
            QJsonArray choicesArray = obj["choices"].toArray();

            for (const QJsonValue& cVal : choicesArray) {
                QJsonObject cObj = cVal.toObject();

                Choice choice;
                choice.text = cObj["text"].toString();
                choice.nextNodeId = cObj["next"].toInt();

                // CONDITIONS
                if (cObj.contains("conditions")) {
                    QJsonObject condObj = cObj["conditions"].toObject();
                    for (auto it = condObj.begin(); it != condObj.end(); ++it)
                        choice.conditions[it.key()] = it.value().toVariant();
                }

                // FLAGS
                if (cObj.contains("setFlags")) {
                    QJsonObject flagObj = cObj["setFlags"].toObject();
                    for (auto it = flagObj.begin(); it != flagObj.end(); ++it)
                        choice.setFlags[it.key()] = it.value().toVariant();
                }

                node.choices.append(choice);
            }
        }

        nodes[node.id] = node;
    }

    qDebug() << "Loaded nodes:" << nodes.size();
}

// ================= NAV =================

void DialogueManager::next()
{
    qDebug() << "NEXT FUNCTION CALLED";

    if (!nodes.contains(currentNodeId)) {
        qDebug() << "Node not found!";
        return;
    }

    int nextId = nodes[currentNodeId].nextNodeId;

    qDebug() << "Current:" << currentNodeId
             << "Next:" << nextId;

    if (nextId != -1)
        setCurrentNode(nextId);
}

// ================= SAVE / LOAD =================

void DialogueManager::saveGame()
{
    QJsonObject saveData;

    saveData["currentNode"] = currentNodeId;

    QJsonObject stateObj;
    for (auto it = m_state.begin(); it != m_state.end(); ++it)
        stateObj[it.key()] = QJsonValue::fromVariant(it.value());

    saveData["state"] = stateObj;

    QFile file("save.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Save failed!";
        return;
    }

    file.write(QJsonDocument(saveData).toJson());
    file.close();

    qDebug() << "Game Saved";
}

void DialogueManager::loadGame()
{
    QFile file("save.json");

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No save file!";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject()) {
        qDebug() << "Invalid save file!";
        return;
    }

    QJsonObject saveData = doc.object();

    currentNodeId = saveData["currentNode"].toInt();

    m_state.clear();

    QJsonObject stateObj = saveData["state"].toObject();
    for (auto it = stateObj.begin(); it != stateObj.end(); ++it)
        m_state[it.key()] = it.value().toVariant();

    setCurrentNode(currentNodeId);

    qDebug() << "Game Loaded";
}

void DialogueManager::restartGame()
{
    m_state.clear();

    if (!nodes.isEmpty())
        setCurrentNode(nodes.firstKey());
}