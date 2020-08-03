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
    if (disabled || too_high || this->timed_out) {
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
    if (0x4000 <= address && address < 0x4004) {
        bool had_lch = this->squares[0].counter_halt();
        bool had_sweep = this->squares[0].sweep_enabled();
        this->squares[0].write(address - 0x4000, data);
        if (address == 0x4003) {
            this->squares[0].timed_out = false;
        }
        bool has_lch = this->squares[0].counter_halt();
        if (!had_lch && has_lch) {
            qDebug() << "Channel 0 length counter halted.";
        }
        if (had_lch && !has_lch) {
            qDebug() << "Channel 0 length counter started.";
        }
        bool has_sweep = this->squares[0].sweep_enabled();
        if (false && !had_sweep && has_sweep) {
            qDebug() << "Sweep Enabled"
                     << this->squares[0].sweep_negate()
                     << this->squares[0].sweep_shift()
                     << this->squares[0].sweep_period();
        }
        if (false && had_sweep && !has_sweep) {
            qDebug() << "Sweep Disabled";
        }
    }
    if (0x4004 <= address && address < 0x4008) {
        bool had_lch = this->squares[1].counter_halt();
        this->squares[1].write(address - 0x4004, data);
        if (address == 0x4007) {
            this->squares[1].timed_out = false;
        }
        bool has_lch = this->squares[1].counter_halt();
        if (!had_lch && has_lch) {
            qDebug() << "Channel 1 length counter halted.";
        }
        if (had_lch && !has_lch) {
            qDebug() << "Channel 1 length counter started.";
        }
    }
    if (0x4008 <= address && address < 0x400c)
        return this->triangle.write(address - 0x4008, data);
    if (address == 0x4015) {
        this->squares[0].enabled = data & 0x01;
        this->squares[1].enabled = data & 0x02;
        this->triangle.enabled = data & 0x04;
    }
    if (address == 0x4017) {
        this->framecounter_mode = (data >> 7) & 0x01;
        //qDebug() << "Frame counter mode set to:" << QString::number(this->framecounter_mode);
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
    bool prev_sweep_enabled = miniapu.squares[0].sweep_enabled();
    bool prev_sweep_negate = miniapu.squares[0].sweep_negate();
    int prev_sweep_shift = miniapu.squares[0].sweep_shift();
    int prev_sweep_period = miniapu.squares[0].sweep_period();
    for (const apu_log_t &entry: apu->apu_log) {
        if (entry.event == apu_log_event::register_write) {
            miniapu.write(entry.address, entry.data);
        } else if (entry.event == apu_log_event::timeout) {
            miniapu.squares[static_cast<int>(entry.channel)].timed_out = true;
        }
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
                    } else if (previous.length < 179 && previous.shape != CycleShape::None) {
                        // If the previous tone was less than 1ms long, it's probably
                        // the result of multiple register writes that only happened
                        // at different times because the NES hardware doesn't let you
                        // write multiple registers simultaneously. Mark these tones
                        // as irregular.
                        //qDebug() << "Marking irregular tone";
                        previous.shape = CycleShape::Irregular;
                    } else {
                        //qDebug() << previous.start << previous.length << previous.semitone_id << previous.volume;
                    }
                    new_tone[channel_i] = true;
                    if (channel_i == 0 && prev_sweep_enabled) {
                        int half_frame = (miniapu.framecounter_mode == 1 ? 37282 : 29830) / 2;
                        int sweep_steps = previous.length / half_frame;
                        int start_nes_timer = previous.nes_timer;
                        int end_nes_timer = start_nes_timer;
                        for (int i=0; i<sweep_steps; i++) {
                            int change_factor = end_nes_timer >> prev_sweep_shift;
                            if (prev_sweep_negate) {
                                change_factor = -1 - change_factor;
                            }
                            end_nes_timer += change_factor;
                        }
                        //qDebug() << "Sweep from" << start_nes_timer << "to" << end_nes_timer;
                    }
                }
            }
            if (new_tone[channel_i] || !has_previous[channel_i]) {
                tones[channel_i].append(tone[channel_i]);
                tone[channel_i] = ToneObject {};
                prev_sweep_enabled = miniapu.squares[0].sweep_enabled();
                prev_sweep_negate = miniapu.squares[0].sweep_negate();
                prev_sweep_shift = miniapu.squares[0].sweep_shift();
                prev_sweep_period = miniapu.squares[0].sweep_period();
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
