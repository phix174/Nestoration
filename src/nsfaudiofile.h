#ifndef NSFAUDIOFILE_H
#define NSFAUDIOFILE_H

#include "audiofile.h"
#include "gme/gme.h"

class NsfAudioFile : public AudioFile
{
    Q_OBJECT

public:
    explicit NsfAudioFile(QObject *parent = 0);
    ~NsfAudioFile();

    void open(QString file_name) override;
    void read_runs() override;
    void process_runs() override;

private:
    Music_Emu *emu;
    const int blipbuf_sample_rate = 6300;
};

#endif // NSFAUDIOFILE_H
