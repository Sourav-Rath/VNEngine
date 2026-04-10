#pragma once

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QList>
#include <QVariant>

#include "Node.h"   // ✅ USE shared data instead of redefining

class EngineController : public QObject {
    Q_OBJECT

public:
    explicit EngineController(QObject *parent = nullptr);

    Q_INVOKABLE QString getCurrentDialogue() const;
    Q_INVOKABLE QList<Choice> getChoices() const;
    Q_INVOKABLE void next();
    Q_INVOKABLE void chooseOption(int index);

signals:
    void dialogueChanged();

private:
    void loadFromJson(const QString& path);

    QMap<int, Node> nodes;
    int currentNodeId = 0;
};