#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QAudioOutput>

#include "generator.h"
#include "toneobject.h"

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 position MEMBER position NOTIFY playerPositionChanged)

public:
    explicit Player(QObject *parent = nullptr);
    void start();

public slots:
    void setChannels(QList<QList<Run>> channel_runs);
    void handleStateChanged(QAudio::State new_state);
    void handlePositionChanged(qint64 byte_position);
    void seek(qint64 sample_position);
    void play_pause();
    void toggle_mute(qint8 channel_i);
    void stop();

signals:
    void playerPositionChanged(qint64 byte_position);

private:
    QAudioOutput *audio;
    Generator *generator;
    qint64 position = 0;

};

#endif // PLAYER_H
