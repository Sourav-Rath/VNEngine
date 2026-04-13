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

    // 🔥 DEFAULT STATE (CRITICAL FIX)
    m_state["karma"] = 0;
    m_state["sanity"] = 0;
    m_state["knowledge"] = 0;
    m_state["violence"] = 0;
    m_state["time"] = 0;

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
        QVariant current = m_state.value(key, 0);

        for (auto op = ops.begin(); op != ops.end(); ++op)
        {
            QString type = op.key();
            QVariant required = op.value();

            if (type == "eq" && !(current == required)) return false;
            if (type == "neq" && !(current != required)) return false;
            if (type == "gte" && !(current.toInt() >= required.toInt())) return false;
            if (type == "lte" && !(current.toInt() <= required.toInt())) return false;
            if (type == "gt" && !(current.toInt() > required.toInt())) return false;
            if (type == "lt" && !(current.toInt() < required.toInt())) return false;
        }
    }

    return true;
}

QString DialogueManager::buildRequirementText(const QVariantMap& condition)
{
    if (condition.contains("logic") && condition.contains("rules"))
    {
        QString logic = condition.value("logic").toString();
        QVariantList rules = condition.value("rules").toList();

        QStringList parts;

        for (const QVariant& rule : rules)
            parts.append(buildRequirementText(rule.toMap()));

        QString joiner = (logic == "AND") ? " AND " : " OR ";

        return "(" + parts.join(joiner) + ")";
    }

    QStringList parts;

    for (auto it = condition.begin(); it != condition.end(); ++it)
    {
        QString key = it.key();
        QVariantMap ops = it.value().toMap();

        for (auto op = ops.begin(); op != ops.end(); ++op)
        {
            QString type = op.key();
            QVariant value = op.value();

            if (type == "gte") parts.append(key + " ≥ " + value.toString());
            else if (type == "lte") parts.append(key + " ≤ " + value.toString());
            else if (type == "gt") parts.append(key + " > " + value.toString());
            else if (type == "lt") parts.append(key + " < " + value.toString());
            else if (type == "eq") parts.append(key);
        }
    }

    return parts.join(" AND ");
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
        QString text = buildRequirementText(choice.conditions);
        choice.requirement = text.isEmpty() ? "Requirements not met" : "Need " + text;
    }
}

void DialogueManager::selectChoice(int index)
{
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

    for (auto it = choice.setFlags.begin(); it != choice.setFlags.end(); ++it)
    {
        setFlag(it.key(), it.value());
    }

    setCurrentNode(choice.nextNodeId);

    m_inputLocked = false;
    emit inputLockedChanged();
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

QVariantMap DialogueManager::getState() const
{
    return m_state;
}

void DialogueManager::setFlag(const QString& key, const QVariant& value)
{
    if (value.typeId() == QMetaType::Bool)
        m_state[key] = value;
    else if (value.canConvert<int>())
        m_state[key] = m_state.value(key, 0).toInt() + value.toInt();
    else
        m_state[key] = value;

    qDebug() << "FLAG SET:" << key << "=" << m_state[key];

    emit stateChanged();
    emit choicesChanged();

    checkFailStates();
}

QVariant DialogueManager::getFlag(const QString& key)
{
    return m_state.value(key, 0);
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

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "[JSON ERROR]" << error.errorString();
        return;
    }

    QJsonObject root = doc.object();

    if (!root.contains("nodes"))
    {
        qDebug() << "[JSON ERROR] Missing 'nodes'";
        return;
    }

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

                    if (condObj.contains("logic"))
                        choice.conditions = condObj.toVariantMap();
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

// ================= FAIL SYSTEM =================

void DialogueManager::checkFailStates()
{
    int sanity = m_state.value("sanity", 0).toInt();
    int time = m_state.value("time", 0).toInt();

    if (sanity <= -3 || time <= -5)
    {
        qDebug() << "FAIL TRIGGERED";
        setCurrentNode(3);
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

    //  RESET DEFAULTS
    m_state["karma"] = 0;
    m_state["sanity"] = 0;
    m_state["knowledge"] = 0;
    m_state["violence"] = 0;
    m_state["time"] = 0;

    QJsonObject stateObj = saveData["state"].toObject();
    for (auto it = stateObj.begin(); it != stateObj.end(); ++it)
        m_state[it.key()] = it.value().toVariant();

    setCurrentNode(currentNodeId);
}

void DialogueManager::restartGame()
{
    m_state.clear();

    //  RESET DEFAULT STATE
    m_state["karma"] = 0;
    m_state["sanity"] = 0;
    m_state["knowledge"] = 0;
    m_state["violence"] = 0;
    m_state["time"] = 0;

    if (!nodes.isEmpty())
        setCurrentNode(nodes.firstKey());

    emit stateChanged();
}