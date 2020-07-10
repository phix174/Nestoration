#include "channelmodel.h"
#include "toneobject.h"

ChannelModel::ChannelModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ChannelModel::ChannelModel(const QVector<ToneObject> tones, QObject *parent)
    : QAbstractListModel(parent), tones(tones)
{
}

void ChannelModel::set_tones(QVector<ToneObject> tones) {
    this->beginResetModel();
    this->tones = tones;
    this->endResetModel();
}

int ChannelModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return this->tones.size();
}

QVariant ChannelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || rowCount() < index.row())
        return QVariant();

    const ToneObject tone = this->tones.at(index.row());

    switch (role) {
        case SemiToneIdRole:
            return QVariant(tone.semitone_id);
        break;
        case DutyRole:
            return QVariant(tone.duty);
        break;
        case LengthRole:
            return QVariant((qint64)tone.length);
        break;
        case NameRole:
            return QVariant(tone.name());
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> ChannelModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SemiToneIdRole] = "semitone_id";
    roles[DutyRole] = "duty";
    roles[LengthRole] = "length";
    roles[NameRole] = "name";
    return roles;
}
