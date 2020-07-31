#ifndef NSFAUDIOFILE_H
#define NSFAUDIOFILE_H

#include "audiofile.h"
#include "gme/gme.h"

class ApuRegisters {
    bool first_write[4] { true, true, true, true };
protected:
    char registers[4] { 0, 0, 0, 0 };
public:
    bool write(short register_i, char data);
};

class SquareRegisters : public ApuRegisters {
public:
    int duty();
    bool counter_halt();
    bool constant_volume();
    int volume();
    bool sweep_enabled();
    int sweep_period();
    bool sweep_negate();
    int sweep_shift();
    int timer_low();
    int length_counter();
    int timer_high();
    int timer_whole();
    double midi_note();
    int out_volume();
    bool enabled { false };
};

class TriangleRegisters : public ApuRegisters {
public:
    bool counter_halt();
    int linear_counter();
    int timer_low();
    int length_counter();
    int timer_high();
    int timer_whole();
    double midi_note();
    bool enabled { false };
};

class MiniApu {
public:
    SquareRegisters square0;
    SquareRegisters square1;
    TriangleRegisters triangle;
    bool write(short address, char data);
};

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
