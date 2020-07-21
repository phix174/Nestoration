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

QVariant ChannelModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() > this->rowCount())
        return QVariant();
    const ToneObject tone = this->tones.at(index.row());
    switch (role) {
        case SemiToneIdRole:
            return QVariant(tone.semitone_id);
        break;
        case ShapeRole:
            return QVariant(tone.shape);
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

Qt::ItemFlags ChannelModel::flags(const QModelIndex &index) const {
    if (!index.isValid() || index.row() > this->rowCount())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool ChannelModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() > this->rowCount())
        return false;
    ToneObject *tone = &this->tones[index.row()];
    bool changed { false };
    switch (role) {
        case SemiToneIdRole:
            changed = (tone->semitone_id != value.toDouble());
            tone->semitone_id = value.toDouble();
        break;
        case ShapeRole:
            changed = (tone->shape != value.toInt());
            tone->shape = value.toInt();
        break;
        case LengthRole:
            changed = (tone->length != value.toInt());
            tone->length = value.toInt();
        break;
    }
    if (changed) emit dataChanged(index, index, {role});
    return true;
}

QHash<int, QByteArray> ChannelModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[SemiToneIdRole] = "semitone_id";
    roles[ShapeRole] = "shape";
    roles[LengthRole] = "length";
    roles[NameRole] = "name";
    return roles;
}
