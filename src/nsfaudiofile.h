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

    int choose_track();
    void open(QString file_name);
    void read_gme_buffer();
    void convert_apulog_to_runs();

signals:
    void GmeBufferChanged(const QByteArray gme_buffer);
    void fileOpened();

public slots:
    void openClicked();

private:
    Music_Emu *emu;
    const int blipbuf_sample_rate;
};

#endif // NSFAUDIOFILE_H
