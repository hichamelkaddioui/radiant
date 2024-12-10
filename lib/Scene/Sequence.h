#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <Arduino.h>
#include <LightChrono.h>
#include <GraphBank.h>

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
    Sequence(uint8_t controlNote);
    Sequence(PlaybackMode mode, GraphOptions graphOptions, uint8_t triggerNote = 0);

    // Graph options
    Graph *_graph = nullptr;
    int _min;
    int _max;
    unsigned long _duration;
    float _period = 1.0f;

    // Playback
    PlaybackMode _mode;

    // Chrono
    unsigned long elapsed() const;
    void restart();

    // MIDI control
    uint8_t _triggerNote;
    uint8_t _controlNote;
    bool _triggered = false;
    void onNotePlayed(uint8_t note, uint8_t velocity, String sequenceName);
    bool isTriggerOn() const { return _triggerNote != 0; }

    // Get the value of the sequence
    int update();

    // Serialization
    size_t serialize(uint8_t *buffer, const GraphBank &graphBank);
    size_t deserialize(const uint8_t *buffer, const GraphBank &graphBank);

    void dump(int ledId, String sequenceName);

private:
    int _value;
    LightChrono _chrono;
};

#endif
