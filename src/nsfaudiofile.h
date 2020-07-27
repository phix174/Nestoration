#ifndef NSFAUDIOFILE_H
#define NSFAUDIOFILE_H

#include "audiofile.h"
#include "gme/gme.h"

class NsfAudioFile : public AudioFile
{
public:
    NsfAudioFile();
    ~NsfAudioFile();

    void open(QString file_name);

private:
    Music_Emu *emu;
};

#endif // NSFAUDIOFILE_H
