#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <Arduino.h>
#include <LightChrono.h>
#include <Graph.h>

enum PlaybackMode
{
    ONCE,
    REPEAT,
    EXTERNAL_CONTROL,
};

class Sequence
{
public:
    int _min;
    int _max;
    unsigned long _duration;
    int _graphId;
    PlaybackMode _mode;

    bool _triggerOn = false;
    uint8_t _triggerNote = 0;
    uint8_t _controlNote = 0;

    Sequence() = default;
    Sequence(int min, int max, unsigned long duration, int graphId, PlaybackMode mode = PlaybackMode::REPEAT) : _min(min), _max(max), _duration(duration), _graphId(graphId), _mode(mode) {}

    void onNotePlayed(uint8_t note, uint8_t velocity);
    int update(const GraphBank &graphBank);

#ifdef DEBUG
    void dump();
#endif

private:
    int _value;
    LightChrono _chrono;
};

#endif