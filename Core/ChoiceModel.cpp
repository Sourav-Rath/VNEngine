#include "ChoiceModel.h"

ChoiceModel::ChoiceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ChoiceModel::setChoices(const QList<Choice> &choices)
{
    beginResetModel();
    m_choices = choices;
    endResetModel();
}

int ChoiceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_choices.size();
}

QVariant ChoiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_choices.size())
        return QVariant();

    const Choice &choice = m_choices[index.row()];

    if (role == TextRole)
        return choice.text;
    else if (role == NextNodeRole)
        return choice.nextNodeId;

    return QVariant();
}

QHash<int, QByteArray> ChoiceModel::roleNames() const
{
    return {
        { TextRole, "text" },
        { NextNodeRole, "nextNodeId" }
    };
}