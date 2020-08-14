#ifndef MINIAPU_H
#define MINIAPU_H

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
    bool timed_out { false };
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
    int out_volume();
    bool enabled { false };
    bool timed_out { false };
    bool timed_out_linear { false };
};

class MiniApu {
public:
    SquareRegisters squares[2];
    TriangleRegisters triangle;
    char framecounter_mode;
    bool write(short address, char data);
};

#endif // MINIAPU_H
