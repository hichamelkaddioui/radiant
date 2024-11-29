#include <Utils.h>
#include <Sequence.h>

Sequence::Sequence(GraphOptions graphOptions, PlaybackMode mode, MidiOptions midiOptions)
    : _graphOptions(graphOptions),
      _mode(mode),
      _triggerNote(midiOptions.triggerNote),
      _controlNote(midiOptions.controlNote)
{
    if (midiOptions.triggerNote != 0)
    {
        _triggerOn = true;
    }
}

void Sequence::onNotePlayed(uint8_t note, uint8_t velocity)
{
    if (_triggerOn && note == _triggerNote)
    {
        _triggered = true;
        _chrono.restart();
    }

    if (note == _controlNote)
    {
        _value = map(velocity, 1, 127, _graphOptions.min, _graphOptions.max);

        debug(1, "[sequence] Control note played %d, velocity %d, value %d", note, velocity, _value);
    }
}

int Sequence::update()
{
    // External control sets the value
    if (_mode == PlaybackMode::EXTERNAL_CONTROL)
    {
        return _value;
    }

    int min = _graphOptions.min;
    int max = _graphOptions.max;
    unsigned long duration = _graphOptions.duration;
    unsigned long elapsed = _chrono.elapsed();
    float period = _graphOptions.period;

    // Wait for a first trigger to play the sequence
    if (_mode == PlaybackMode::ONCE && _triggerOn && !_triggered)
    {
        // Return the end value before the first trigger
        elapsed = duration;
    }

    // Sequence duration is over
    if (elapsed >= duration)
    {
        // In loop mode restart the Sequence
        if (_mode == PlaybackMode::REPEAT)
        {
            _chrono.restart();

            return update();
        }

        // In once mode return the end value
        elapsed = duration;
    }

    float t = (float)elapsed / (float)duration;
    Graph *graph = _graphOptions.graph;

    if (graph == nullptr)
    {
        debug(1, "[sequence] No graph");
        return _value;
    }

    PeriodicGraph *periodicGraph = dynamic_cast<PeriodicGraph *>(graph);
    if (periodicGraph != nullptr && 0 < period)
    {
        periodicGraph->_period = period;
    }

    float value = graph->valueAt(t);

    // Min and max scaling
    int amplitude = max - min;
    _value = min + (int)(value * amplitude);

    return _value;
}

void Sequence::dump()
{
    Graph *graph = _graphOptions.graph;
    int min = _graphOptions.min;
    int max = _graphOptions.max;
    unsigned long duration = _graphOptions.duration;
    float period = _graphOptions.period;
    PlaybackMode mode = _mode;

    String modeString;

    switch (mode)
    {
    case PlaybackMode::REPEAT:
        modeString = "REPEAT";
        break;
    case PlaybackMode::ONCE:
        modeString = "ONCE";
        break;
    case PlaybackMode::EXTERNAL_CONTROL:
        modeString = "EXTERNAL_CONTROL";
        break;
    default:
        modeString = "UNKNOWN";
        break;
    }

    debug(1, "[sequence] graph %p\tmin %d\t\tmax %d\t\tduration %d\tperiod %f\tmode %s", graph, min, max, duration, period, modeString.c_str());
}

void Sequence::restart() { _chrono.restart(); }

unsigned long Sequence::elapsed() const { return _chrono.elapsed(); }
