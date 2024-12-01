#ifndef IO_MIDI_H
#define IO_MIDI_H

#include <SPI.h>
#include <Wire.h>
#include <MIDI.h>
#include <Oled.h>
#include <Scene.h>

USING_NAMESPACE_MIDI

class MidiSerial
{
public:
    void setup();
    void loop(SceneBank &sceneBank);

private:
    void handleNoteOn(SceneBank &sceneBank);
    void handleSystemExclusive(SceneBank &sceneBank);
    void handleProgramChange(SceneBank &sceneBank);
    void handleControlChange(SceneBank &sceneBank);
};

#endif