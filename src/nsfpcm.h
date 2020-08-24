#ifndef NSFPCM_H
#define NSFPCM_H

#include <QIODevice>

class Music_Emu;

class NsfPcm : public QIODevice
{
    Q_OBJECT

public:
    NsfPcm(const int output_rate);
    ~NsfPcm();

    void set_emu(Music_Emu *emu, qreal length_sec);

    void set_mute(uint8_t channel_i, int muted);

    bool seek_sample(qint64 sample_position);
    bool atEnd() const override;
    qint64 readData(char *data, qint64 bytes_requested) override;
    qint64 writeData(const char *data, qint64 len) override;

signals:
    void positionChanged(qint64 byte_position);

private:
    int output_rate;
    Music_Emu *emu;
    qreal length_sec;
};

#endif // NSFPCM_H
