#ifndef NODE_H
#define NODE_H

#include <QString>
#include <QList>

// =========================
// Choice Struct
// =========================
struct Choice
{
    QString text;
    int nextNodeId;
};

// =========================
// Node Struct
// =========================
struct Node
{
    QString text;
    int nextNodeId;
    QList<Choice> choices;
};

#endif // NODE_H