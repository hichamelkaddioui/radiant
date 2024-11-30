#include <Utils.h>
#include <Sequence.h>

Sequence::Sequence(GraphOptions graphOptions, PlaybackMode mode, MidiOptions midiOptions)
    : _graph(graphOptions.graph),
      _min(graphOptions.min),
      _max(graphOptions.max),
      _duration(graphOptions.duration),
      _period(graphOptions.period),
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
        _value = map(velocity, 1, 127, _min, _max);

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

    int min = _min;
    int max = _max;
    unsigned long duration = _duration;
    unsigned long elapsed = _chrono.elapsed();
    float period = _period;

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
    Graph *graph = _graph;

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

void Sequence::restart() { _chrono.restart(); }

unsigned long Sequence::elapsed() const { return _chrono.elapsed(); }

size_t Sequence::serialize(uint8_t *buffer, const GraphBank &graphBank)
{
    size_t offset = 0;

    int graphId = graphBank.getGraphId(_graph);

    if (graphId == -1)
    {
        debug(1, "[serialize sequence] graph not found, graph address %p", _graph);

        return 0;
    }

    memcpy(buffer + offset, &graphId, sizeOfInt);
    offset += sizeOfInt;

    memcpy(buffer + offset, &_min, sizeOfInt);
    offset += sizeOfInt;
    memcpy(buffer + offset, &_max, sizeOfInt);
    offset += sizeOfInt;
    memcpy(buffer + offset, &_duration, sizeOfLong);
    offset += sizeOfLong;
    memcpy(buffer + offset, &_period, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(buffer + offset, &_mode, sizeof(PlaybackMode));
    offset += sizeof(PlaybackMode);
    memcpy(buffer + offset, &_triggerNote, sizeOfByte);
    offset += sizeOfByte;
    memcpy(buffer + offset, &_controlNote, sizeOfByte);
    offset += sizeOfByte;

    debug(1, "[serialize sequence] trigger note %d, control note %d", _triggerNote, _controlNote);
    debug(1, "[serialize sequence] mode %d", _mode);
    debug(1, "[serialize sequence] graph min %d, max %d, duration %d, period %f", _min, _max, _duration, _period);

    return offset;
}

size_t Sequence::deserialize(const uint8_t *buffer, const GraphBank &graphBank)
{
    size_t offset = 0;

    int graphId = 0;
    memcpy(&graphId, buffer + offset, sizeOfInt);
    offset += sizeOfInt;

    _graph = graphBank._bank.at(graphId);

    if (_graph == nullptr)
    {
        debug(1, "[deserialize sequence] graph not found");

        return 0;
    }

    memcpy(&_min, buffer + offset, sizeOfInt);
    offset += sizeOfInt;
    memcpy(&_max, buffer + offset, sizeOfInt);
    offset += sizeOfInt;
    memcpy(&_duration, buffer + offset, sizeOfLong);
    offset += sizeOfLong;
    memcpy(&_period, buffer + offset, sizeOfFloat);
    offset += sizeOfFloat;
    memcpy(&_mode, buffer + offset, sizeof(PlaybackMode));
    offset += sizeof(PlaybackMode);
    memcpy(&_triggerNote, buffer + offset, sizeOfByte);
    offset += sizeOfByte;
    memcpy(&_controlNote, buffer + offset, sizeOfByte);
    offset += sizeOfByte;

    debug(1, "[deserialize sequence] trigger note %d, control note %d", _triggerNote, _controlNote);
    debug(1, "[deserialize sequence] mode %d", _mode);
    debug(1, "[deserialize sequence] graph min %d, max %d, duration %d, period %f", _min, _max, _duration, _period);

    return offset;
}

void Sequence::dump()
{
    Graph *graph = _graph;
    int min = _min;
    int max = _max;
    unsigned long duration = _duration;
    float period = _period;
    PlaybackMode mode = _mode;
    uint8_t triggerNote = _triggerNote;
    uint8_t controlNote = _controlNote;

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

    debug(1, "[sequence] trigger note %u, control note %u, mode %s, graph min %d, max %d, duration %d, period %f", triggerNote, controlNote, modeString.c_str(), min, max, duration, period);
}
