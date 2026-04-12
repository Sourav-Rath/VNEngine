#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

struct Choice
{
    QString text;
    int nextNodeId = -1;

    bool isEnabled = true;
    QString requirement;

    QMap<QString, QVariant> conditions;
    QMap<QString, QVariant> setFlags;

    QList<QVariantMap> events;
};

struct Node
{
    int id = -1;   // ✅ THIS WAS MISSING

    QString text;

    QList<Choice> choices;

    int nextNodeId = -1;

    QList<QVariantMap> events;
};