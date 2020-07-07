#ifndef GENERATOR_H
#define GENERATOR_H

#include <QIODevice>
#include <QVector>

class Run;

class Generator : public QIODevice
{

public:
    Generator();

    qint64 render_runs(QByteArray *buffer, qint64 maxSize);
    void convert_buffer(const QByteArray in, float out[]);
    size_t resample_soxr(float in[], float out[], size_t size);

    bool seek(qint64 pos) override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    QVector<Run> runs;

    qint64 run_i;
    qint64 run_i_sample;
};

#endif // GENERATOR_H
