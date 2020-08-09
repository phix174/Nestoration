#include <QDebug>

#include "nsfpcm.h"
#include "gme/gme.h"

NsfPcm::NsfPcm(const int output_rate)
    : output_rate(output_rate)
{

}

NsfPcm::~NsfPcm() {
}

void NsfPcm::set_emu(Music_Emu *emu) {
    this->emu = emu;
}

void NsfPcm::set_mute(uint8_t channel_i, int muted) {
    gme_mute_voice(this->emu, channel_i, muted);
}

bool NsfPcm::seek_sample(qint64 sample_position) {
    gme_seek_samples(this->emu, sample_position);
    return true;
}

qint64 NsfPcm::readData(char *data, qint64 bytes_requested) {
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
