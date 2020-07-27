#include "nsfaudiofile.h"

#include <QDebug>
#include <QInputDialog>

NsfAudioFile::NsfAudioFile()
{
    file_types = "NSF/NSFe (*.nsf *.nsfe)";
}

NsfAudioFile::~NsfAudioFile()
{
    gme_delete(this->emu);
}

void NsfAudioFile::open(QString file_name) {
    const int SAMPLE_RATE = 6300;
    const int STEREO = 2;
    const int SECONDS = 100;
    if (file_name == "") {
        return;
    }
    if (this->is_open) {
        this->close();
    }
    gme_err_t open_err = gme_open_file(qPrintable(file_name), &this->emu, SAMPLE_RATE);
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
    if (!valid_choice) {
        return;
    }
    qDebug() << "Track" << track_num << "selected";
    gme_start_track(this->emu, track_num);
    short *buf = new short[SAMPLE_RATE * STEREO * SECONDS];
    gme_play(this->emu, SAMPLE_RATE * STEREO * SECONDS, buf);
    delete[] buf;
    // TODO: set is_open if track is open
    if (!this->is_open) {
        this->close();
    }
}
