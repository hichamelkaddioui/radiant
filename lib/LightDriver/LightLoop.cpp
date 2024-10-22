#include <LightLoop.h>

void LightLoop::setup(KeyframeList *hueKeyframesList, KeyframeList *valueKeyframesList, KeyframeList *saturationKeyframesList)
{
    _color = {0, 0, 0};

    _hueState.keyframesList = hueKeyframesList;
    _valueState.keyframesList = valueKeyframesList;
    _saturationState.keyframesList = saturationKeyframesList;
}

void LightLoop::loop()
{
    updateStateValue(&_hueState, &_color.hue);
    updateStateValue(&_valueState, &_color.val);
    updateStateValue(&_saturationState, &_color.sat);
}

void LightLoop::updateStateValue(LightLoopState *state, uint8_t *value)
{
    if (state == nullptr)
        return;

    int keyframeCount = state->keyframesList->_count;

    if (keyframeCount == 0)
        return;

    Keyframe lastKeyframe = state->keyframesList->_keyframes[keyframeCount - 1];

    bool isLastKeyframeOver = state->chrono.hasPassed(lastKeyframe.time, true);

    if (isLastKeyframeOver)
        return;

    unsigned long elapsed = state->chrono.elapsed();

    *value = static_cast<uint8_t>(state->keyframesList->update(elapsed));
}
