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
        ShapeRole,
        LengthRole,
        NameRole
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;
private:
    QVector<ToneObject> tones;
};

#endif // CHANNELMODEL_H
