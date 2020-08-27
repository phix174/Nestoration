#include "nsfaudiofile.h"
#include "miniapu.h"
#include "channelmodel.h"
#include "gme/Nsf_Emu.h"

#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>

const int INVALID_TRACK = -1;

NsfAudioFile::NsfAudioFile(int sample_rate, QObject *parent)
    : AudioFile(parent), blipbuf_sample_rate(sample_rate)
{
    file_types = "NSF/NSFe (*.nsf *.NSF *.nsfe *.NSFE)";
}

NsfAudioFile::~NsfAudioFile() {
    if (this->emu) {
        gme_delete(this->emu);
    }
}

void NsfAudioFile::openClicked() {
    QSettings settings;
    QString starting_dir = settings.value("open_dir", QDir::homePath()).toString();
    QString file_name = QFileDialog::getOpenFileName(nullptr, "Open a NES music file", starting_dir, this->file_types);
    if (file_name == "") {
        return;
    }
    settings.setValue("open_dir", QFileInfo(file_name).dir().canonicalPath());
    this->open(file_name);
}

void NsfAudioFile::open(QString file_name) {
    if (this->is_open) {
        this->close();
        this->file_track = INVALID_TRACK;
    }
    gme_err_t open_err = gme_open_file(qPrintable(file_name), &this->emu, this->blipbuf_sample_rate);
    if (open_err) {
        qDebug() << open_err;
        return;
    }
    QString file_name_only = QFileInfo(file_name).fileName();
    emit this->fileOpened(file_name_only);
    this->list_tracks();
}

void NsfAudioFile::list_tracks() {
    int track_count = gme_track_count(this->emu);
    qDebug() << "Track count:" << track_count;
    QStringList tracks;
    QList<int> track_lengths;
    for (int i = 0; i < track_count; i += 1) {
        gme_info_t *track_info;
        gme_track_info(this->emu, &track_info, i);
        QString title = track_info->song;
        QString option = QString::number(i + 1) + (title.isEmpty() ? "" : ": " + title);
        tracks.append(option);
        track_lengths.append(track_info->play_length);
    }
    emit this->tracksListed(tracks, track_lengths);
}

void NsfAudioFile::select_track(qint16 track_num, qreal length_sec) {
    if (track_num != INVALID_TRACK && track_num < 256) {
        qDebug() << "Track" << track_num << "selected";
        gme_enable_accuracy(this->emu, 1);
        Nes_Apu *apu = static_cast<Nsf_Emu*>(this->emu)->apu_();
        apu->apu_log_enabled = true;
        gme_err_t start_err = gme_start_track(this->emu, track_num);
        if (!start_err) {
            this->is_open = true;
            this->file_track = track_num;
        }
    }
    if (!this->is_open) {
        this->close();
        return;
    }
    this->read_gme_buffer(length_sec);
    this->convert_apulog_to_runs(length_sec);
    gme_seek_samples(this->emu, 0);
    emit this->emuChanged(this->emu, length_sec);
    emit this->trackOpened(this->file_track);
}

void NsfAudioFile::read_gme_buffer(qreal length_sec) {
    const int STEREO = 2;
    int length = this->blipbuf_sample_rate * STEREO * (length_sec + 1);
    short *buf = new short[length];
    Nes_Apu *apu = static_cast<Nsf_Emu*>(this->emu)->apu_();
    gme_play(this->emu, length, buf);
    apu->apu_log_enabled = false;
    delete[] buf;
}

void NsfAudioFile::convert_apulog_to_runs(qreal length_sec) {
    Nes_Apu *apu = static_cast<Nsf_Emu*>(this->emu)->apu_();
    MiniApu miniapu;
    QVector<ToneObject> tones[3];
    bool has_previous[3] { false, false, false };
    bool new_tone[3] { false, false, false };
    sampleoff last_sample = length_sec * 1789773; /* TODO: Don't hard-code the CPU frequency. */
    ToneObject tone[3];
    for (int channel_i = 0; channel_i < 3; channel_i += 1) {
        tone[channel_i].start = apu->apu_log.first().cpu_cycle;
    }
    /* TODO: Take sweep into account when deciding whether two tones are different. */
    /* TODO: Take envelope into account when deciding whether two tones are different. */
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
        } else if (entry.event == apu_log_event::sweep && entry.cpu_cycle < last_sample) {
            // TODO: The last_sample condition above is not the best way
            // to make cut-off sweeping tones render accurately.
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
                has_previous[channel_i] = true;
                new_tone[channel_i] = false;
            }
        }
    }
    for (int channel_i = 0; channel_i < 3; channel_i += 1) {
        ToneObject &final_tone = tones[channel_i].last();
        while (final_tone.start >= last_sample) {
            tones[channel_i].removeLast();
            final_tone = tones[channel_i].last();
        }
        ToneObject filler_tone;
        if (final_tone.length == 0 || final_tone.start + final_tone.length > last_sample) {
            filler_tone = final_tone;
            tones[channel_i].removeLast();
        }
        sampleoff filled = filler_tone.start;
        while (filled < last_sample) {
            // Qt crashes if a tone is 2^26 samples long or longer, so we need to make several smaller tones.
            filler_tone.start = filled;
            filler_tone.length = 1789773;
            if (filler_tone.start + filler_tone.length > last_sample) {
                filler_tone.length = last_sample - filler_tone.start;
            }
            filled += filler_tone.length;
            tones[channel_i].append(filler_tone);
        }
    }
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
