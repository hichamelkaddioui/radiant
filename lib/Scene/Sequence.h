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

struct GraphOptions
{
    Graph *graph = nullptr;
    int min;
    int max;
    unsigned long duration;
    float period = 1.0f;
};

class Sequence
{
public:
    Sequence() = default;
    Sequence(Graph *graph, int min, int max, unsigned long duration, PlaybackMode mode, float period = 1.0f, uint8_t triggerNote = 0, uint8_t controlNote = 0);

    // Graph options
    GraphOptions _graphOptions;

    // Playback
    PlaybackMode _mode;

    // Chrono
    unsigned long elapsed() const;
    void restart();

    // MIDI control
    uint8_t _triggerNote;
    uint8_t _controlNote;
    bool _triggerOn = false;
    bool _triggered = false;
    void onNotePlayed(uint8_t note, uint8_t velocity);

    // Get the value of the sequence
    int update();

#ifdef DEBUG
    void dump();
#endif

private:
    int _value;
    LightChrono _chrono;
};

#endif
