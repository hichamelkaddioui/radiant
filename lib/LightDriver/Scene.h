#include <vector>
#include <LightChrono.h>
#include <Utils.h>

#ifndef SCENE_H
#define SCENE_H

#ifndef DEFAULT_POWER_VALUE
#define DEFAULT_POWER_VALUE 1.0f
#endif

#ifndef DEFAULT_PERIOD
#define DEFAULT_PERIOD 100
#endif

typedef enum
{
    LINEAR,
    EASE,
    WAVE,
    GATE,
} CurveType;

union CurveCoefficient
{
    float powerValue;
    struct
    {
        float min;
        float max;
        unsigned long period;
    } rangeAndPeriod;

    CurveCoefficient(){};
    CurveCoefficient(float powerValue) : powerValue(powerValue) {}
    CurveCoefficient(float min, float max, unsigned long period) : rangeAndPeriod({min, max, period}) {}
};

struct Curve
{
    CurveType type;
    CurveCoefficient coefficient;

    Curve(CurveType type = CurveType::LINEAR, CurveCoefficient coefficient = CurveCoefficient()) : type(type), coefficient(coefficient) {}

    static Curve linear()
    {
        return Curve{LINEAR};
    }

    static Curve ease(float powerValue = DEFAULT_POWER_VALUE)
    {
        return Curve{EASE, CurveCoefficient(powerValue)};
    }

    static Curve wave(float min = -10.f, float max = 10.f, unsigned long period = DEFAULT_PERIOD)
    {
        return Curve{WAVE, CurveCoefficient(min, max, period)};
    }

    static Curve gate(float min = 0.f, float max = 1.f, unsigned long period = DEFAULT_PERIOD)
    {
        return Curve{GATE, CurveCoefficient(min, max, period)};
    }
};

struct Keyframe
{
    unsigned long time;
    float value;
    Curve curve;

    Keyframe(unsigned long time = 0, float value = 0, Curve curve = Curve()) : time(time), value(value), curve(curve) {}
};

typedef enum
{
    LOOP,
    TRIGGER
} SceneMode;

class Scene
{
public:
    Scene() = default;
    Scene(std::vector<Keyframe> keyframes, SceneMode mode = SceneMode::LOOP) : _keyframes(keyframes), _mode(mode)
    {
        _chrono.restart();

#ifdef DEBUG
        for (auto it = _keyframes.begin(); it != _keyframes.end(); ++it)
        {
            if (it->curve.type == CurveType::EASE)
                debug(1, "[scene] keyframe: %d\t%f\tease\tpw=%f", it->time, it->value, it->curve.coefficient.powerValue);
            if (it->curve.type == CurveType::WAVE)
                debug(1, "[scene] keyframe: %d\t%f\twave\tmin=%f\tmax=%f\tperiod=%lu", it->time, it->value, it->curve.coefficient.rangeAndPeriod.min, it->curve.coefficient.rangeAndPeriod.max, it->curve.coefficient.rangeAndPeriod.period);
            if (it->curve.type == CurveType::GATE)
                debug(1, "[scene] keyframe: %d\t%f\tgate\tmin=%f\tmax=%f\tperiod=%lu", it->time, it->value, it->curve.coefficient.rangeAndPeriod.min, it->curve.coefficient.rangeAndPeriod.max, it->curve.coefficient.rangeAndPeriod.period);
            if (it->curve.type == CurveType::LINEAR)
                debug(1, "[scene] keyframe: %d\t%f\tlinear", it->time, it->value);
        }
#endif
    };
    float update();
    void trigger();

private:
    std::vector<Keyframe> _keyframes;
    SceneMode _mode;
    LightChrono _chrono;
    float interpolate(unsigned long currentTime, unsigned long totalTime, float startValue, float endValue, Curve curve);
};

#endif