#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <iostream>

class AudioFile
{
public:
    AudioFile();

    void open(const char *file_name);
    void read_block(char block[], std::streamsize &bytes_read);
    void close();

private:
    struct archive *m_archive;
};

#endif // AUDIOFILE_H
