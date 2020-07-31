#include "nsfaudiofile.h"
#include "channelmodel.h"
#include "gme/Nsf_Emu.h"
#include "gme/Nes_Apu.h"

#include <QDebug>
#include <QInputDialog>

bool ApuRegisters::write(short register_i, char data) {
    char old_data = this->registers[register_i];
    this->registers[register_i] = data;
    if (this->first_write[register_i]) {
        this->first_write[register_i] = false;
        return true;
    }
    return data != old_data;
}

int SquareRegisters::duty() { return (this->registers[0] >> 6) & 0x03; }
bool SquareRegisters::counter_halt() { return (this->registers[0] >> 5) & 0x01; }
bool SquareRegisters::constant_volume() { return (this->registers[0] >> 4) & 0x01; }
int SquareRegisters::volume() { return (this->registers[0] >> 0) & 0x0f; }
bool SquareRegisters::sweep_enabled() { return (this->registers[1] >> 7) & 0x01; }
int SquareRegisters::sweep_period() { return (this->registers[1] >> 4) & 0x07; }
bool SquareRegisters::sweep_negate() { return (this->registers[1] >> 3) & 0x01; }
int SquareRegisters::sweep_shift() { return (this->registers[1] >> 0) & 0x07; }
int SquareRegisters::timer_low() { return (this->registers[2] >> 0) & 0xff; }
int SquareRegisters::length_counter() { return (this->registers[3] >> 3) & 0x1f; }
int SquareRegisters::timer_high() { return (this->registers[3] >> 0) & 0x07; }
int SquareRegisters::timer_whole() { return (this->timer_high() << 8) + this->timer_low(); }
double SquareRegisters::midi_note() {
    int period = 16 * (this->timer_whole() + 1);
    return period_to_semitone(period);
}
int SquareRegisters::out_volume() {
    bool disabled = !this->enabled;
    bool too_high = this->timer_whole() < 8;
    if (disabled || too_high) {
        return 0;
    }
    return this->volume();
}

bool TriangleRegisters::counter_halt() { return (this->registers[0] >> 7) & 0x01; }
int TriangleRegisters::linear_counter() { return (this->registers[0] >> 1) & 0x7f; };
int TriangleRegisters::timer_low() { return (this->registers[2] >> 0) & 0xff; };
int TriangleRegisters::length_counter() { return (this->registers[3] >> 3) & 0x1f; };
int TriangleRegisters::timer_high() { return (this->registers[3] >> 0) & 0x07; };
int TriangleRegisters::timer_whole() { return (this->timer_high() << 8) + this->timer_low(); }
double TriangleRegisters::midi_note() {
    int period = 32 * (this->timer_whole() + 1);
    return period_to_semitone(period);
}

bool MiniApu::write(short address, char data) {
    if (0x4000 <= address && address < 0x4004)
        return this->squares[0].write(address - 0x4000, data);
    if (0x4004 <= address && address < 0x4008)
        return this->squares[1].write(address - 0x4004, data);
    if (0x4008 <= address && address < 0x400c)
        return this->triangle.write(address - 0x4008, data);
    if (address == 0x4015) {
        this->squares[0].enabled = data & 0x01;
        this->squares[1].enabled = data & 0x02;
        this->triangle.enabled = data & 0x04;
    }
    return false;
}

NsfAudioFile::NsfAudioFile(QObject *parent)
    : AudioFile(parent)
{
    file_types = "NSF/NSFe (*.nsf *.nsfe)";
}

NsfAudioFile::~NsfAudioFile()
{
    gme_delete(this->emu);
}

void NsfAudioFile::open(QString file_name) {
    if (file_name == "") {
        return;
    }
    if (this->is_open) {
        this->close();
    }
    gme_err_t open_err = gme_open_file(qPrintable(file_name), &this->emu, this->blipbuf_sample_rate);
    qDebug() << open_err;
    int track_count = gme_track_count(this->emu);
    qDebug() << "Track count:" << track_count;
    bool ok;
    QStringList tracks;
    for (int i = 0; i < track_count; i += 1) {
        tracks.append(QString::number(i + 1));
    }
    QString track_num_str = QInputDialog::getItem(nullptr, tr("Choose a track to open"), tr("Track:"), tracks, 0, false, &ok);
    int track_num = track_num_str.toInt() - 1;
    bool valid_choice = (ok && 0 <= track_num && track_num < track_count);
    if (valid_choice) {
        qDebug() << "Track" << track_num << "selected";
        gme_err_t start_err = gme_start_track(this->emu, track_num);
        if (!start_err) {
            this->is_open = true;
        }
        gme_info_t *track_info;
        gme_track_info(this->emu, &track_info, track_num);
        qDebug() << "Track Length:" << track_info->play_length << "ms";
    }
    if (!this->is_open) {
        this->close();
    }
}

void NsfAudioFile::read_runs() {
    const int STEREO = 2;
    const int SECONDS = 100;
    short *buf = new short[this->blipbuf_sample_rate * STEREO * SECONDS];
    gme_play(this->emu, this->blipbuf_sample_rate * STEREO * SECONDS, buf);
    delete[] buf;
    Nes_Apu *apu = static_cast<Nsf_Emu*>(this->emu)->apu_();
    MiniApu miniapu;
    QVector<ToneObject> tones[3];
    bool has_previous[3] { false, false, false };
    bool new_tone[3] { false, false, false };
    ToneObject tone[3];
    for (int channel_i = 0; channel_i < 3; channel_i += 1) {
        tone[channel_i].start = apu->apu_log.first().cpu_cycle;
    }
    for (const apu_log_t &entry: apu->apu_log) {
        miniapu.write(entry.address, entry.data);
        for (int channel_i = 0; channel_i < 3; channel_i += 1) {
            if (channel_i < 2) {
                tone[channel_i].semitone_id = miniapu.squares[channel_i].midi_note();
                tone[channel_i].nes_timer = miniapu.squares[channel_i].timer_whole();
                tone[channel_i].volume = miniapu.squares[channel_i].out_volume();
                tone[channel_i].shape = tone[channel_i].volume ? miniapu.squares[channel_i].duty() + 1 : CycleShape::None;
            } else {
                tone[channel_i].semitone_id = miniapu.triangle.midi_note();
                tone[channel_i].nes_timer = miniapu.triangle.timer_whole();
                tone[channel_i].volume = tone[channel_i].nes_timer > 2 ? 15 : 0;
                tone[channel_i].shape = tone[channel_i].volume ? CycleShape::Triangle : CycleShape::None;
            }
            if (has_previous[channel_i]) {
                ToneObject &previous = tones[channel_i].last();
                if (tone[channel_i].semitone_id != previous.semitone_id
                        || tone[channel_i].shape != previous.shape
                        || tone[channel_i].volume != previous.volume) {
                    tone[channel_i].start = entry.cpu_cycle;
                    previous.length = tone[channel_i].start - previous.start;
                    if (previous.length == 0) {
                        // If the current tone and the previous tone started on the same CPU cycle
                        // (such as cycle 0), then replace the previous tone with the current tone.
                        qDebug() << "Deleting length-0 tone";
                        tones[channel_i].removeLast();
                    } else {
                        qDebug() << previous.start << previous.length << previous.semitone_id << previous.volume;
                    }
                    new_tone[channel_i] = true;
                }
            }
            if (new_tone[channel_i] || !has_previous[channel_i]) {
                tones[channel_i].append(tone[channel_i]);
                tone[channel_i] = ToneObject {};
                has_previous[channel_i] = true;
                new_tone[channel_i] = false;
            }
        }
    }
    // Discard the last tone because we don't know how long it is.
    tones[0].removeLast();
    tones[1].removeLast();
    tones[2].removeLast();
    this->channel0->set_tones(tones[0]);
    this->channel1->set_tones(tones[1]);
    this->channel2->set_tones(tones[2]);
    emit this->channel0Changed(this->channel0);
    emit this->channel1Changed(this->channel1);
    emit this->channel2Changed(this->channel2);
    this->highest_tone = -999;
    this->lowest_tone = 999;
    this->determine_range(tones[0]);
    this->determine_range(tones[1]);
    this->determine_range(tones[2]);
    emit this->lowestToneChanged(this->lowest_tone);
    emit this->highestToneChanged(this->highest_tone);
}

void NsfAudioFile::process_runs() {
    // pass
}
