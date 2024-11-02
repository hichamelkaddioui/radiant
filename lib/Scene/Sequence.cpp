#include <Utils.h>
#include <Sequence.h>

void Sequence::onNotePlayed(uint8_t note, uint8_t velocity)
{
    if (_triggerOn && note == _triggerNote)
    {
        _chrono.restart();
    }

    if (note == _controlNote)
    {
        _value = map(velocity, 0, 127, _min, _max);
    }
}

int Sequence::update(const GraphBank &graphBank)
{
    // External control sets the value
    if (_mode == PlaybackMode::EXTERNAL_CONTROL)
    {
        return _value;
    }

    unsigned long elapsed = _chrono.elapsed();

    // Sequence duration is over
    if (elapsed >= _duration)
    {
        // In loop mode restart the Sequence
        if (_mode == PlaybackMode::REPEAT)
        {
            _chrono.restart();

            return update(graphBank);
        }

        // In once mode return the last value
        elapsed = _duration;
    }

    // Sequence duration is not over
    float t = (float)elapsed / (float)_duration;
    Graph *graph = graphBank.at(_graphId);
    if (graph == nullptr)
    {
        return _value;
    }
    float value = graph->valueAt(t);

    // Min and max scaling
    int amplitude = _max - _min;
    _value = _min + (int)(value * amplitude);

    return _value;
}

void Sequence::dump()
{
    String mode;

    switch (_mode)
    {
    case PlaybackMode::REPEAT:
        mode = "REPEAT";
        break;
    case PlaybackMode::ONCE:
        mode = "ONCE";
        break;
    default:
    case PlaybackMode::EXTERNAL_CONTROL:
        mode = "EXTERNAL_CONTROL";
        break;
    }

    debug(1, "[sequence] graph %d, min %d, max %d, duration %d, mode %s, triggerOn %d, triggerNote %d, controlNote %d", _graphId, _min, _max, _duration, mode.c_str(), _triggerOn, _triggerNote, _controlNote);
}