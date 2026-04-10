#ifndef NODE_H
#define NODE_H

#include <QString>
#include <QList>
#include <QVariantMap>

// =========================
// Choice Struct
// =========================
struct Choice {
    QString text;
    int nextNodeId;

    QVariantMap setFlags;
    QVariantMap conditions;

    QList<QVariantMap> events;

    bool isEnabled = true;
    QString requirementText;
};

// =========================
// Node Struct
// =========================
struct Node
{
    QString text;
    int nextNodeId;
    QList<Choice> choices;
    QList<QVariantMap> events;
};

#endif // NODE_H