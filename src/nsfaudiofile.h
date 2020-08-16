#ifndef NSFAUDIOFILE_H
#define NSFAUDIOFILE_H

#include "audiofile.h"
#include "gme/gme.h"

class NsfAudioFile : public AudioFile
{
    Q_OBJECT

public:
    explicit NsfAudioFile(int sample_rate, QObject *parent = 0);
    ~NsfAudioFile();

    void open(QString file_name);
    void list_tracks();
    void read_gme_buffer();
    void convert_apulog_to_runs();

signals:
    void emuChanged(Music_Emu *emu);
    void fileOpened(QString file_name);
    void tracksListed(QStringList tracks, QList<int> track_lengths);
    void trackOpened(qint16 file_track);

public slots:
    void openClicked();
    void select_track(qint16 track_num, qint32 length_msec);

private:
    Music_Emu *emu { nullptr };
    const int blipbuf_sample_rate;
    qint16 file_track = -1;
};

#endif // NSFAUDIOFILE_H
