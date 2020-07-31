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
        return this->square0.write(address - 0x4000, data);
    if (0x4004 <= address && address < 0x4008)
        return this->square1.write(address - 0x4004, data);
    if (0x4008 <= address && address < 0x400c)
        return this->triangle.write(address - 0x4008, data);
    if (address == 0x4015) {
        this->square0.enabled = data & 0x01;
        this->square1.enabled = data & 0x02;
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
    ToneObject sq1tone;
    sq1tone.start = apu->apu_log.first().cpu_cycle;
    QVector<ToneObject> tones;
    bool has_previous = false;
    bool new_tone = false;
    for (const apu_log_t &entry: apu->apu_log) {
        miniapu.write(entry.address, entry.data);
        sq1tone.semitone_id = miniapu.square0.midi_note();
        sq1tone.nes_timer = miniapu.square0.timer_whole();
        sq1tone.volume = miniapu.square0.out_volume();
        sq1tone.shape = sq1tone.volume ? miniapu.square0.duty() + 1 : CycleShape::None;
        if (has_previous) {
            ToneObject &previous = tones.last();
            if (sq1tone.semitone_id != previous.semitone_id
                    || sq1tone.shape != previous.shape
                    || sq1tone.volume != previous.volume) {
                sq1tone.start = entry.cpu_cycle;
                previous.length = sq1tone.start - previous.start;
                if (previous.length == 0) {
                    // If the current tone and the previous tone started on the same CPU cycle
                    // (such as cycle 0), then replace the previous tone with the current tone.
                    qDebug() << "Deleting length-0 tone";
                    tones.removeLast();
                } else {
                    qDebug() << previous.start << previous.length << previous.semitone_id << previous.volume;
                }
                new_tone = true;
            }
        }
        if (new_tone || !has_previous) {
            tones.append(sq1tone);
            sq1tone = ToneObject {};
            has_previous = true;
            new_tone = false;
        }
    }
    // Discard the last tone because we don't know how long it is.
    tones.removeLast();
    this->channel0->set_tones(tones);
    emit this->channel0Changed(this->channel0);
    this->highest_tone = -999;
    this->lowest_tone = 999;
    this->determine_range(tones);
    emit this->lowestToneChanged(this->lowest_tone);
    emit this->highestToneChanged(this->highest_tone);
}

void NsfAudioFile::process_runs() {
    // pass
}
