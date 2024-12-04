#ifndef IO_MIDI_H
#define IO_MIDI_H

#include <SPI.h>
#include <Wire.h>
#include <MIDI.h>
#include <Oled.h>
#include <Scene.h>

USING_NAMESPACE_MIDI

enum SysExMessage
{
    SET_PARAMS = 1,
    CREATE_LIGHT = 2,
    CREATE_SCENE = 3,
    CREATE_GRAPH = 4,
    SET_HUE_A = 5,
    SET_BRIGHTNESS_A = 6,
    SET_HUE_B = 7,
    SET_BRIGHTNESS_B = 8,
    SET_STROBE_A = 9,
    SET_STROBE_B = 10,
};

class MidiSerial
{
public:
    void setup();
    void loop(LedBank &ledBank, GraphBank &graphBank, SceneBank &sceneBank);

private:
    void handleNoteOn(SceneBank &sceneBank);
    void handleSystemExclusive(LedBank &ledBank, GraphBank &graphBank, SceneBank &sceneBank);
    void handleProgramChange(SceneBank &sceneBank);
    void handleControlChange(SceneBank &sceneBank);
};

#endif