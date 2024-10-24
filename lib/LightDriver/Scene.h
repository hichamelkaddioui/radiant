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
    unsigned long period;

    CurveCoefficient(float powerValue = DEFAULT_POWER_VALUE) : powerValue(powerValue) {}
    CurveCoefficient(int period = DEFAULT_PERIOD) : period(period) {}
};

struct Curve
{
    CurveType type;
    CurveCoefficient coefficient;

    Curve(CurveType type = CurveType::LINEAR, CurveCoefficient coefficient = CurveCoefficient(DEFAULT_POWER_VALUE)) : type(type), coefficient(coefficient) {}
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
    Scene(std::vector<Keyframe> keyframes, SceneMode mode = SceneMode::TRIGGER) : _keyframes(keyframes), _mode(mode)
    {
        if (keyframes.size() < 2)
        {
            _keyframes.push_back(Keyframe(0, 0.0f, Curve(CurveType::EASE, CurveCoefficient(2.0f))));

            if (keyframes.size() < 2)
            {
                _keyframes.push_back(Keyframe(0, 0.0f, Curve(CurveType::EASE, CurveCoefficient(2.0f))));
            }
        }

        for (auto it = _keyframes.begin(); it != _keyframes.end(); ++it)
        {
            debug(1, "[scene] Keyframe: (%lu, %f), curve: {pw: %f, ct: %lu, type: %d}", it->time, it->value, it->curve.coefficient.powerValue, it->curve.coefficient.period, it->curve.type);
        }
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