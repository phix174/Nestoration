#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>
#include <QVector>
#include "soxr.h"

struct Run;

struct Channel {
    QList<Run> runs;
    qint64 runs_i;
    qint64 runs_i_sample;
    uint8_t *buffer;
};

class AudioFile;

class Generator : public QIODevice
{

public:
    Generator(AudioFile &audio_file, const int output_rate);
    ~Generator();

    void init_soxr();
    void setChannels(QList<Run> (&channel_runs)[5]);
    qint64 render_runs(Channel &channel, qint64 maxSize);
    void mix_channels(qint64 size);
    size_t resample_soxr(float out[], size_t size);

    bool seek_sample(qint64 sample_position);
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    AudioFile *audio_file;
    int output_rate;
    Channel channels[5];
    float *mixed_buffer = nullptr;
    float *downsampled_buffer = nullptr;
    soxr_t soxr = nullptr;
};

#endif // GENERATOR_H
