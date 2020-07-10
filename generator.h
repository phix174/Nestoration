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

class Generator : public QIODevice
{

public:
    Generator();
    ~Generator();

    void setChannels(QList<Run> (&channel_runs)[5]);
    qint64 render_runs(Channel &channel, qint64 maxSize);
    void mix_channels(qint64 size);
    size_t resample_soxr(float out[], size_t size);

    bool seek(qint64 pos) override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    Channel channels[5];
    float *mixed_buffer;
    soxr_t soxr;
};

#endif // GENERATOR_H
