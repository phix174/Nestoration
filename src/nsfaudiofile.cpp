#include "nsfaudiofile.h"
#include "miniapu.h"
#include "channelmodel.h"
#include "gme/Nsf_Emu.h"

#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>

const int INVALID_TRACK = -1;

NsfAudioFile::NsfAudioFile(int sample_rate, QObject *parent)
    : AudioFile(parent), blipbuf_sample_rate(sample_rate)
{
    file_types = "NSF/NSFe (*.nsf *.NSF *.nsfe *.NSFE)";
}

NsfAudioFile::~NsfAudioFile()
{
    gme_delete(this->emu);
}

void NsfAudioFile::openClicked()
{
    QString nes_dir = QDir::homePath() + QString("/storage/audio/emu/nes");
    QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a NES music file", nes_dir, this->file_types);
    //QString file_name = QDir::homePath() + QString("/storage/audio/emu/nes/Disney's DuckTales (Released Version) (NTSC) (SFX).nsf");
    if (file_name == "") {
        return;
    }
    QString file_name_only = QFileInfo(file_name).fileName();
    this->open(file_name);
    if (this->is_open) {
        this->read_gme_buffer();
        this->convert_apulog_to_runs();
        emit this->fileOpened(file_name_only, this->file_track);
        gme_seek_samples(this->emu, 0);
        emit this->emuChanged(this->emu);
    }
}

int NsfAudioFile::choose_track() {
    int track_count = gme_track_count(this->emu);
    qDebug() << "Track count:" << track_count;
    bool ok;
    QStringList tracks;
    for (int i = 0; i < track_count; i += 1) {
        gme_info_t *track_info;
        gme_track_info(this->emu, &track_info, i);
        QString title = track_info->song;
        tracks.append(QString::number(i + 1) + (title.isEmpty() ? "" : ": " + title));
    }
    QString track_num_str = QInputDialog::getItem(nullptr, tr("Choose a track to open"), tr("Track:"), tracks, 0, false, &ok);
    if (!ok) {
        return INVALID_TRACK;
    }
    int track_num = tracks.indexOf(track_num_str);
    if (track_num >= track_count) {
        return INVALID_TRACK;
    }
    return track_num;
}

void NsfAudioFile::open(QString file_name) {
    if (this->is_open) {
        this->close();
        this->file_track = INVALID_TRACK;
    }
    gme_err_t open_err = gme_open_file(qPrintable(file_name), &this->emu, this->blipbuf_sample_rate);
    qDebug() << open_err;
    int track_num = this->choose_track();
    if (track_num != INVALID_TRACK) {
        qDebug() << "Track" << track_num << "selected";
        gme_enable_accuracy(this->emu, 1);
        gme_err_t start_err = gme_start_track(this->emu, track_num);
        if (!start_err) {
            this->is_open = true;
            this->file_track = track_num;
        }
        gme_info_t *track_info;
        gme_track_info(this->emu, &track_info, track_num);
        qDebug() << "Track Length:" << track_info->play_length << "ms";
    }
    if (!this->is_open) {
        this->close();
    }
}

void NsfAudioFile::read_gme_buffer() {
    const int STEREO = 2;
    const int SECONDS = 100;
    int length = this->blipbuf_sample_rate * STEREO * SECONDS;
    short *buf = new short[length];
    gme_play(this->emu, length, buf);
    int byte_length = length * sizeof(short);
    QByteArray blip_byte_array { reinterpret_cast<char*>(buf), byte_length };
    delete[] buf;
}

void NsfAudioFile::convert_apulog_to_runs() {
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
    //int prev_sweep_period = miniapu.squares[0].sweep_period();
    short sweep_end[2] { -1, -1 };
    std::sort(apu->apu_log.begin(), apu->apu_log.end());
    for (const apu_log_t &entry: apu->apu_log) {
        if (entry.event == apu_log_event::register_write) {
            miniapu.write(entry.address, entry.data);
        } else if (entry.event == apu_log_event::timeout) {
            if (entry.channel < 2) {
                miniapu.squares[static_cast<int>(entry.channel)].timed_out = true;
            } else {
                miniapu.triangle.timed_out = true;
            }
        } else if (entry.event == apu_log_event::timeout_linear) {
            miniapu.triangle.timed_out_linear = true;
        } else if (entry.event == apu_log_event::reloaded_linear) {
            miniapu.triangle.timed_out_linear = false;
        } else if (entry.event == apu_log_event::sweep) {
            sweep_end[static_cast<int>(entry.channel)] = entry.data;
        }
        for (int channel_i = 0; channel_i < 3; channel_i += 1) {
            if (channel_i < 2) {
                tone[channel_i].nes_timer = miniapu.squares[channel_i].timer_whole();
                tone[channel_i].semitone_id = period_to_semitone(16 * (tone[channel_i].nes_timer + 1));
                tone[channel_i].volume = miniapu.squares[channel_i].out_volume();
                tone[channel_i].shape = tone[channel_i].volume ? miniapu.squares[channel_i].duty() + 1 : CycleShape::None;
            } else {
                tone[channel_i].nes_timer = miniapu.triangle.timer_whole();
                tone[channel_i].semitone_id = period_to_semitone(32 * (tone[channel_i].nes_timer + 1));
                tone[channel_i].volume = miniapu.triangle.out_volume();
                tone[channel_i].shape = tone[channel_i].volume ? CycleShape::Triangle : CycleShape::None;
            }
            if (has_previous[channel_i]) {
                ToneObject &previous = tones[channel_i].last();
                if (tone[channel_i].nes_timer != previous.nes_timer
                        || tone[channel_i].shape != previous.shape
                        || tone[channel_i].volume != previous.volume) {
                    tone[channel_i].start = entry.cpu_cycle;
                    previous.length = tone[channel_i].start - previous.start;
                    if (channel_i < 2 && sweep_end[channel_i] > -1) {
                        previous.nes_timer_end = sweep_end[channel_i];
                        sweep_end[channel_i] = -1;
                    }
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
                }
            }
            if (new_tone[channel_i] || !has_previous[channel_i]) {
                tones[channel_i].append(tone[channel_i]);
                tone[channel_i] = ToneObject {};
                prev_sweep_enabled = miniapu.squares[0].sweep_enabled();
                prev_sweep_negate = miniapu.squares[0].sweep_negate();
                prev_sweep_shift = miniapu.squares[0].sweep_shift();
                //prev_sweep_period = miniapu.squares[0].sweep_period();
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
