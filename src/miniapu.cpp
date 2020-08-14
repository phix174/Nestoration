#include "miniapu.h"
#include "toneobject.h"

#include <QDebug>

bool ApuRegisters::write(short register_i, char data) {
    char old_data = this->registers[register_i];
    this->registers[register_i] = data;
    if (this->first_write[register_i]) {
        this->first_write[register_i] = false;
        return true;
    }
    return data != old_data;
}

int SquareRegisters::duty() { return (this->registers[0] >> 6) & 0x03; }
bool SquareRegisters::counter_halt() { return (this->registers[0] >> 5) & 0x01; }
bool SquareRegisters::constant_volume() { return (this->registers[0] >> 4) & 0x01; }
int SquareRegisters::volume() { return (this->registers[0] >> 0) & 0x0f; }
bool SquareRegisters::sweep_enabled() { return (this->registers[1] >> 7) & 0x01; }
int SquareRegisters::sweep_period() { return (this->registers[1] >> 4) & 0x07; }
bool SquareRegisters::sweep_negate() { return (this->registers[1] >> 3) & 0x01; }
int SquareRegisters::sweep_shift() { return (this->registers[1] >> 0) & 0x07; }
int SquareRegisters::timer_low() { return (this->registers[2] >> 0) & 0xff; }
int SquareRegisters::length_counter() { return (this->registers[3] >> 3) & 0x1f; }
int SquareRegisters::timer_high() { return (this->registers[3] >> 0) & 0x07; }
int SquareRegisters::timer_whole() { return (this->timer_high() << 8) + this->timer_low(); }
int SquareRegisters::out_volume() {
    bool disabled = !this->enabled;
    bool too_high = this->timer_whole() < 8;
    if (disabled || too_high || this->timed_out) {
        return 0;
    }
    return this->volume();
}

bool TriangleRegisters::counter_halt() { return (this->registers[0] >> 7) & 0x01; }
int TriangleRegisters::linear_counter() { return (this->registers[0] >> 1) & 0x7f; };
int TriangleRegisters::timer_low() { return (this->registers[2] >> 0) & 0xff; };
int TriangleRegisters::length_counter() { return (this->registers[3] >> 3) & 0x1f; };
int TriangleRegisters::timer_high() { return (this->registers[3] >> 0) & 0x07; };
int TriangleRegisters::timer_whole() { return (this->timer_high() << 8) + this->timer_low(); }
int TriangleRegisters::out_volume() {
    bool disabled = !this->enabled;
    bool too_high = this->timer_whole() < 2;
    if (disabled || too_high || this->timed_out || this->timed_out_linear) {
        return 0;
    }
    return 15;
}

bool MiniApu::write(short address, char data) {
    if (0x4000 <= address && address < 0x4004) {
        bool had_lch = this->squares[0].counter_halt();
        bool had_sweep = this->squares[0].sweep_enabled();
        this->squares[0].write(address - 0x4000, data);
        if (address == 0x4003) {
            this->squares[0].timed_out = false;
        }
        bool has_lch = this->squares[0].counter_halt();
        if (!had_lch && has_lch) {
            qDebug() << "Channel 0 length counter halted.";
        }
        if (had_lch && !has_lch) {
            qDebug() << "Channel 0 length counter started.";
        }
        bool has_sweep = this->squares[0].sweep_enabled();
        if (false && !had_sweep && has_sweep) {
            qDebug() << "Sweep Enabled"
                     << this->squares[0].sweep_negate()
                     << this->squares[0].sweep_shift()
                     << this->squares[0].sweep_period();
        }
        if (false && had_sweep && !has_sweep) {
            qDebug() << "Sweep Disabled";
        }
    }
    if (0x4004 <= address && address < 0x4008) {
        bool had_lch = this->squares[1].counter_halt();
        this->squares[1].write(address - 0x4004, data);
        if (address == 0x4007) {
            this->squares[1].timed_out = false;
        }
        bool has_lch = this->squares[1].counter_halt();
        if (!had_lch && has_lch) {
            qDebug() << "Channel 1 length counter halted.";
        }
        if (had_lch && !has_lch) {
            qDebug() << "Channel 1 length counter started.";
        }
    }
    if (0x4008 <= address && address < 0x400c) {
        this->triangle.write(address - 0x4008, data);
        if (address == 0x400b) {
            this->triangle.timed_out = false;
        }
    }
    if (address == 0x4015) {
        this->squares[0].enabled = data & 0x01;
        this->squares[1].enabled = data & 0x02;
        this->triangle.enabled = data & 0x04;
    }
    if (address == 0x4017) {
        this->framecounter_mode = (data >> 7) & 0x01;
        //qDebug() << "Frame counter mode set to:" << QString::number(this->framecounter_mode);
    }
    return false;
}
