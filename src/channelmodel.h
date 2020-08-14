#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QAbstractListModel>

#include "toneobject.h"

class ChannelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ChannelModel(QObject *parent = nullptr);
    explicit ChannelModel(const QVector<ToneObject> tones, QObject *parent = nullptr);

    void set_tones(QVector<ToneObject> tones);

    enum ModelRoles {
        SemiToneIdRole = Qt::UserRole +1,
        SemiToneIdEndRole,
        NesTimerRole,
        NesTimerEndRole,
        ShapeRole,
        StartRole,
        LengthRole,
        VolumeRole,
        NameRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QHash<int, QByteArray> roleNames() const override;
private:
    QVector<ToneObject> tones;
};

#endif // CHANNELMODEL_H
