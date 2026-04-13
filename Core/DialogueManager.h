#pragma once

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QQueue>
#include <QTimer>

#include "Node.h"
#include "ChoiceModel.h"

class DialogueManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentText READ currentText NOTIFY dialogueChanged)
    Q_PROPERTY(ChoiceModel* choicesModel READ choicesModel NOTIFY choicesChanged)
    Q_PROPERTY(bool inputLocked READ inputLocked NOTIFY inputLockedChanged)
    Q_PROPERTY(QVariantMap state READ getState NOTIFY stateChanged)
    Q_PROPERTY(int timer READ timer NOTIFY timerChanged)

public:
    explicit DialogueManager(QObject *parent = nullptr);

    QString buildRequirementText(const QVariantMap& condition);
    QString currentText() const;
    ChoiceModel* choicesModel();

    Q_INVOKABLE void next();
    Q_INVOKABLE void selectChoice(int index);

    Q_INVOKABLE void saveGame();
    Q_INVOKABLE void loadGame();
    Q_INVOKABLE void restartGame();

    Q_INVOKABLE void setFlag(const QString& key, const QVariant& value);
    Q_INVOKABLE QVariant getFlag(const QString& key);

    QVariantMap getState() const;

    int timer() const;

    bool inputLocked() const { return m_inputLocked; }

signals:
    void dialogueChanged();
    void choicesChanged();
    void inputLockedChanged();

    void eventPrint(QString message);
    void eventLog(QString message);
    void eventSound(QString file);

    void timerChanged();
    void stateChanged();

private:
    void setCurrentNode(int nodeId);
    void evaluateChoice(Choice& choice);

    bool evaluateCondition(const QVariantMap& condition);
    bool evaluateConditionGroup(const QVariantMap& group);

    void executeEvents(const QList<QVariantMap>& events);
    void processNextEvent();

    void loadFromJson(const QString &path);

    void checkFailStates();
    void checkWarningStates();


    void startTimer();
    void handleTimeout();

private:
    QMap<int, Node> nodes;
    int currentNodeId = 0;

    ChoiceModel m_choiceModel;
    QMap<QString, QVariant> m_state;

    QQueue<QVariantMap> eventQueue;
    bool m_inputLocked = false;

    QTimer m_timer;
    int m_timeLeft = 5;

    QList<DelayedEffect> m_delayedEffects;
    void processDelayedEffects();
};