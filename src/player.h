#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QBuffer>
#include <QAudioOutput>

#include "generator.h"
#include "nsfpcm.h"
#include "toneobject.h"

class Music_Emu;

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 position MEMBER position NOTIFY playerPositionChanged)

public:
    explicit Player(QAudioFormat out_format, QObject *parent = nullptr);
    ~Player();
    void start();

public slots:
    void setChannels(QList<QList<Run>> channel_runs);
    void setEmu(Music_Emu *emu);
    void handleNotify();
    void handleStateChanged(QAudio::State new_state);
    void handlePositionChanged(qint64 byte_position);
    void seek(qint64 sample_position);
    void play_pause();
    void pause();
    void toggle_mute(qint8 channel_i);
    void stop();

signals:
    void playerPositionChanged(qint64 byte_position);

private:
    QAudioFormat out_format;
    qreal byte_usec_ratio;
    QAudioOutput *audio;
    Generator *generator;
    NsfPcm *nsf_pcm;
    qint64 position = 0;
    qint64 bytes_played = 0;
    qint64 seek_offset = 0;
    int mute_states[5] { 0, 0, 0, 0, 0 };
};

#endif // PLAYER_H
