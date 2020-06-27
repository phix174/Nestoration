#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <iostream>

class AudioFile
{
public:
    AudioFile(const char *file_name);

    void read_block(char block[], std::streamsize &bytes_read);

private:
    struct archive *m_archive;
};

#endif // AUDIOFILE_H
