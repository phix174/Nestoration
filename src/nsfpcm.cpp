#include <QDebug>

#include "nsfpcm.h"
#include "gme/gme.h"

NsfPcm::NsfPcm(const int output_rate)
    : output_rate(output_rate)
{

}

NsfPcm::~NsfPcm() {
}

void NsfPcm::set_emu(Music_Emu *emu, qreal length_sec) {
    this->emu = emu;
    this->length_sec = length_sec;
}

void NsfPcm::set_mute(uint8_t channel_i, int muted) {
    gme_mute_voice(this->emu, channel_i, muted);
}

bool NsfPcm::seek_sample(qint64 sample_position) {
    const short STEREO = 2;
    int stereo_sample_position = sample_position * STEREO;
    gme_seek_samples(this->emu, stereo_sample_position);
    qint64 byte_position = stereo_sample_position * sizeof(short);
    this->seek(byte_position);
    return true;
}

bool NsfPcm::atEnd() const {
    const short STEREO = 2;
    qreal pos_sec = 1.0 * this->pos() / (sizeof(short) * STEREO) / this->output_rate;
    return pos_sec >= this->length_sec;
}

qint64 NsfPcm::readData(char *data, qint64 bytes_requested) {
    const short STEREO = 2;
    qreal end_pos_sec = 1.0 * (this->pos() + bytes_requested) / (sizeof(short) * STEREO) / this->output_rate;
    if (end_pos_sec >= this->length_sec) {
        bytes_requested = this->length_sec * sizeof(short) * STEREO * this->output_rate - this->pos();
    }
    int shorts_requested = bytes_requested / sizeof(short);
    gme_play(this->emu, shorts_requested, reinterpret_cast<short*>(data));
    return bytes_requested;
}

qint64 NsfPcm::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}
