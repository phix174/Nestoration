#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <QObject>
#include <iosfwd>
#include <cmath>

typedef uint8_t samplevalue;
typedef long sampleoff;
typedef long samplesize;

struct Cycle {
    sampleoff start;
    samplesize length;
};

const double CPU_FREQENCY = 1789773.0;
const double TWELFTH_ROOT = pow(2.0, 1.0 / 12.0);
const double A0 = 440.0 / pow(2, 4);
const double C0 = A0 * pow(TWELFTH_ROOT, -9.0);

struct WAVheader {
    char RIFF_literal[4];
    uint32_t chunk_size;
    char WAVE_literal[4];
    char fmt__literal[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_literal[4];
    uint32_t subchunk2_size;
};

class ChannelModel;
class ToneObject;

class AudioFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ChannelModel *channel0 MEMBER channel0 NOTIFY modelChanged)

public:
    explicit AudioFile(QObject *parent = 0);

    void open(const char *file_name);
    void read_block(char block[], std::streamsize &bytes_read);
    void close();
    QVector<Cycle> read_cycles();
    QVector<ToneObject> find_tones(QVector<Cycle> &cycles);

public slots:
    void openClicked();

signals:
    void modelChanged(ChannelModel *channel0);

private:
    struct archive *m_archive;
    ChannelModel *channel0;
};

#endif // AUDIOFILE_H
