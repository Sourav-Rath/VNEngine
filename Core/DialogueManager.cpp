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

// ================= CONDITION SYSTEM =================

bool DialogueManager::evaluateConditionGroup(const QVariantMap& group)
{
    QString logic = group.value("logic", "AND").toString();
    QVariantList rules = group.value("rules").toList();

    if (rules.isEmpty())
        return true;

    if (logic == "AND")
    {
        for (const QVariant& ruleVar : rules)
        {
            if (!evaluateCondition(ruleVar.toMap()))
                return false;
        }
        return true;
    }
    else if (logic == "OR")
    {
        for (const QVariant& ruleVar : rules)
        {
            if (evaluateCondition(ruleVar.toMap()))
                return true;
        }
        return false;
    }

    return true;
}

bool DialogueManager::evaluateCondition(const QVariantMap& condition)
{
    if (condition.contains("logic") && condition.contains("rules"))
        return evaluateConditionGroup(condition);

    for (auto it = condition.begin(); it != condition.end(); ++it)
    {
        QString key = it.key();
        QVariantMap ops = it.value().toMap();
        QVariant current = m_state.value(key, QVariant());

        for (auto op = ops.begin(); op != ops.end(); ++op)
        {
            QString type = op.key();
            QVariant required = op.value();

            bool passed = true;

            if (type == "eq") passed = (current == required);
            else if (type == "neq") passed = (current != required);
            else if (type == "gte") passed = (current.toInt() >= required.toInt());
            else if (type == "lte") passed = (current.toInt() <= required.toInt());
            else if (type == "gt") passed = (current.toInt() > required.toInt());
            else if (type == "lt") passed = (current.toInt() < required.toInt());

            if (!passed)
                return false;
        }
    }

    return true;
}

// ================= CHOICE =================

void DialogueManager::evaluateChoice(Choice& choice)
{
    choice.isEnabled = true;
    choice.requirement = "";

    if (choice.conditions.isEmpty())
        return;

    if (!evaluateCondition(choice.conditions))
    {
        choice.isEnabled = false;
        choice.requirement = "Requirements not met";
    }
}

void DialogueManager::selectChoice(int index)
{
    // HARD LOCK FIRST
    if (m_inputLocked)
        return;

    m_inputLocked = true;
    emit inputLockedChanged();

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

    // EVENTS
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
        m_inputLocked = false;
        emit inputLockedChanged();
        return;
    }

    QVariantMap ev = eventQueue.dequeue();
    QString type = ev["type"].toString();

    if (type == "print")
    {
        emit eventPrint(ev["message"].toString());
    }
    else if (type == "log")
    {
        emit eventLog(ev["message"].toString());
    }
    else if (type == "sound")
    {
        emit eventSound(ev["file"].toString());
    }
    else if (type == "delay")
    {
        int time = ev["time"].toInt();

        QTimer::singleShot(time, this, [this]() {
            processNextEvent();
        });
        return;
    }
    else if (type == "transition")
    {
        setCurrentNode(ev["nextNode"].toInt());
    }

    QTimer::singleShot(0, this, [this]() {
        processNextEvent();
    });
}

// ================= STATE =================

void DialogueManager::setFlag(const QString& key, const QVariant& value)
{
    if (value.typeId() == QMetaType::Bool)
    {
        m_state[key] = value;
    }
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

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    QJsonObject root = doc.object();
    QJsonArray array = root["nodes"].toArray();

    for (const QJsonValue& val : array)
    {
        QJsonObject obj = val.toObject();

        Node node;
        node.id = obj["id"].toInt();
        node.text = obj["text"].toString();
        node.nextNodeId = obj.contains("next") ? obj["next"].toInt() : -1;

        if (obj.contains("choices"))
        {
            QJsonArray choicesArray = obj["choices"].toArray();

            for (const QJsonValue& cVal : choicesArray)
            {
                QJsonObject cObj = cVal.toObject();

                Choice choice;
                choice.text = cObj["text"].toString();
                choice.nextNodeId = cObj["next"].toInt();

                if (cObj.contains("conditions"))
                {
                    QJsonObject condObj = cObj["conditions"].toObject();

                    // 🔥 IMPORTANT: SUPPORT BOTH SIMPLE + ADVANCED
                    if (condObj.contains("logic"))
                    {
                        choice.conditions = condObj.toVariantMap();
                    }
                    else
                    {
                        for (auto it = condObj.begin(); it != condObj.end(); ++it)
                            choice.conditions[it.key()] = it.value().toObject().toVariantMap();
                    }
                }

                if (cObj.contains("setFlags"))
                {
                    QJsonObject flagObj = cObj["setFlags"].toObject();
                    for (auto it = flagObj.begin(); it != flagObj.end(); ++it)
                        choice.setFlags[it.key()] = it.value().toVariant();
                }

                node.choices.append(choice);
            }
        }

        nodes[node.id] = node;
    }
}

// ================= NAV =================

void DialogueManager::next()
{
    if (!nodes.contains(currentNodeId))
        return;

    int nextId = nodes[currentNodeId].nextNodeId;

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
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(saveData).toJson());
    file.close();
}

void DialogueManager::loadGame()
{
    QFile file("save.json");

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
        return;

    QJsonObject saveData = doc.object();

    currentNodeId = saveData["currentNode"].toInt();

    m_state.clear();

    QJsonObject stateObj = saveData["state"].toObject();
    for (auto it = stateObj.begin(); it != stateObj.end(); ++it)
        m_state[it.key()] = it.value().toVariant();

    setCurrentNode(currentNodeId);
}

void DialogueManager::restartGame()
{
    m_state.clear();

    if (!nodes.isEmpty())
        setCurrentNode(nodes.firstKey());
}