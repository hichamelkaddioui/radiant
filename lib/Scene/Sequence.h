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

    bool _triggerOn;
    bool _triggered = false;
    uint8_t _triggerNote;
    uint8_t _controlNote;

    Sequence() = default;
    Sequence(int min, int max, unsigned long duration, int graphId, PlaybackMode mode = PlaybackMode::REPEAT, bool triggerOn = false, uint8_t triggerNote = 0, uint8_t controlNote = 0)
        : _min(min), _max(max), _duration(duration), _graphId(graphId), _mode(mode), _triggerOn(triggerOn), _triggerNote(triggerNote), _controlNote(controlNote) {}

    void onNotePlayed(uint8_t note, uint8_t velocity);
    int update(const GraphBank &graphBank);

    void restart();
    unsigned long elapsed() const;

#ifdef DEBUG
    void dump();
#endif

private:
    int _value;
    LightChrono _chrono;
};

#endif