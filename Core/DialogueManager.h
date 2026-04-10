#pragma once

#include <QObject>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>

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

    //  STATE SYSTEM
    Q_INVOKABLE void setFlag(const QString& key, const QVariant& value);
    Q_INVOKABLE QVariant getFlag(const QString& key) const;

signals:
    void dialogueChanged();
    void choicesChanged();

private:
    void loadFromJson(const QString& path);
    void setCurrentNode(int nodeId);
    bool evaluateChoice(Choice& choice);

    QMap<int, Node> nodes;
    int currentNodeId = 0;

    ChoiceModel m_choiceModel;

    QVariantMap m_state;
};