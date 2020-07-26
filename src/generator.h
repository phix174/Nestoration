#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>
#include <QVector>
#include "soxr.h"
#include "toneobject.h"

struct Channel {
    QList<Run> runs;
    qint64 runs_i;
    qint64 runs_i_sample;
    samplevalue *buffer;
    bool muted;
};

class AudioFile;

class Generator : public QIODevice
{
    Q_OBJECT

public:
    Generator(const int output_rate);
    ~Generator();

    void init_soxr();
    void setChannels(QList<QList<Run>> channel_runs);
    void toggle_mute(uint8_t channel_i);
    qint64 render_runs(Channel &channel, qint64 maxSize);
    void mix_channels(qint64 size);
    size_t resample_soxr(float out[], size_t in_size);

    bool seek_sample(qint64 sample_position);
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

signals:
    void positionChanged(qint64 byte_position);

private:
    int internal_rate = 1789773;
    qreal resolution_multiplier = 1;
    int output_rate;
    qreal sample_rate_ratio;
    Channel channels[5];
    float *mixed_buffer = nullptr;
    float *downsampled_buffer = nullptr;
    soxr_t soxr = nullptr;
};

#endif // GENERATOR_H
