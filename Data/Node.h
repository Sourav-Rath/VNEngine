#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

struct DelayedEffect
{
    QVariantMap effects;
    int turnsRemaining;
};

struct Choice
{
    QString text;
    int nextNodeId = -1;

    bool isEnabled = true;
    QString requirement;

    QMap<QString, QVariant> conditions;
    QMap<QString, QVariant> setFlags;

    QList<QVariantMap> events;
    QList<DelayedEffect> delayedEffects;
};

struct Node
{
    int id = -1;

    QString text;

    QList<Choice> choices;

    int nextNodeId = -1;

    QList<QVariantMap> events;
};