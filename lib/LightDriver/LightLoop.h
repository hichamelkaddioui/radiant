#include <LightChrono.h>
#include <Keyframe.h>

#ifndef LIGHTLOOP_H
#define LIGHTLOOP_H

typedef struct
{
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
} CHSV;

typedef struct
{
    KeyframeList *keyframesList;
    LightChrono chrono;
} LightLoopState;

class LightLoop
{
public:
    void setup(KeyframeList *hueKeyframesList, KeyframeList *valueKeyframesList, KeyframeList *saturationKeyframesList);
    void loop();
    void setColor(CHSV color) { _color = color; }

private:
    CHSV _color;
    LightLoopState _hueState;
    LightLoopState _valueState;
    LightLoopState _saturationState;

    void updateStateValue(LightLoopState *state, uint8_t *value);
};

#endif