#pragma once

#include <QObject>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QQueue>

#include "Node.h"
#include "ChoiceModel.h"

class DialogueManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentText READ currentText NOTIFY dialogueChanged)
    Q_PROPERTY(ChoiceModel* choicesModel READ choicesModel NOTIFY choicesChanged)

public:
    explicit DialogueManager(QObject *parent = nullptr);

    QString currentText() const;
    ChoiceModel* choicesModel();

    Q_INVOKABLE void next();
    Q_INVOKABLE void selectChoice(int index);

    // STATE SYSTEM
    Q_INVOKABLE void setFlag(const QString& key, const QVariant& value);
    Q_INVOKABLE QVariant getFlag(const QString& key) const;

    // SAVE / LOAD / RESTART
    Q_INVOKABLE void saveGame();
    Q_INVOKABLE void loadGame();
    Q_INVOKABLE void restartGame();

signals:
    void dialogueChanged();
    void choicesChanged();

    // EVENT SIGNALS
    void eventPrint(QString message);
    void eventLog(QString message);
    void eventSound(QString file);

private:
    void loadFromJson(const QString& path);
    void setCurrentNode(int nodeId);
    bool evaluateChoice(Choice& choice);

    // EVENT SYSTEM
    void executeEvents(const QList<QVariantMap>& events);
    void processNextEvent();

    QQueue<QVariantMap> eventQueue;

    QMap<int, Node> nodes;
    int currentNodeId = 0;

    ChoiceModel m_choiceModel;

    QVariantMap m_state;
};