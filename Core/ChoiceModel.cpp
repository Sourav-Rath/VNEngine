#include "ChoiceModel.h"

ChoiceModel::ChoiceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void ChoiceModel::setChoices(const QList<Choice> &choices)
{
    beginResetModel();
    m_choices = choices;
    emit choicesChanged();
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

    switch (role)
    {
    case TextRole:
        return choice.text;

    case EnabledRole:
        return choice.isEnabled;

    case RequirementRole:
        return choice.requirement;

    case SetFlagsRole:
        return choice.setFlags;

    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChoiceModel::roleNames() const
{
    return {
        { TextRole, "text" },
        { EnabledRole, "enabled" },
        { RequirementRole, "requirement" },
        { SetFlagsRole, "setFlags" }
    };
}