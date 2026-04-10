#ifndef CHOICEMODEL_H
#define CHOICEMODEL_H

#include <QAbstractListModel>
#include "Node.h"

class ChoiceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        NextNodeRole
    };

    explicit ChoiceModel(QObject *parent = nullptr);

    void setChoices(const QList<Choice> &choices);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<Choice> m_choices;
};

#endif