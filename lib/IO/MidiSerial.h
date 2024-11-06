#ifndef IO_MIDI_H
#define IO_MIDI_H

#include <SPI.h>
#include <Wire.h>
#include <MIDI.h>
#include <Oled.h>

USING_NAMESPACE_MIDI

class MidiSerial
{
public:
    void setup();
    void loop();
};

#endif