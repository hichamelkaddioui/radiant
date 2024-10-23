#include <vector>
#include <LightChrono.h>
#include <Utils.h>

#ifndef SCENE_H
#define SCENE_H

#ifndef DEFAULT_POWER_VALUE
#define DEFAULT_POWER_VALUE 1.0f
#endif

#ifndef DEFAULT_CUTOFF_TIME
#define DEFAULT_CUTOFF_TIME 100
#endif

typedef enum
{
    LINEAR,
    EASE,
    GATE,
} CurveType;

struct CurveCoefficients
{
    float powerValue;
    unsigned long cutoffTime;

    CurveCoefficients(float powerValue = DEFAULT_POWER_VALUE, unsigned long cutoffTime = DEFAULT_CUTOFF_TIME) : powerValue(powerValue), cutoffTime(cutoffTime) {}
};

struct Curve
{
    CurveType type;
    CurveCoefficients coefficients;

    Curve(CurveType type = CurveType::LINEAR, CurveCoefficients coefficients = CurveCoefficients()) : type(type), coefficients(coefficients) {}
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
            // TODO: pad with dummy keyframes
        }

        for (auto it = _keyframes.begin(); it != _keyframes.end(); ++it)
        {
            debug(1, "[scene] Keyframe: (%lu, %f), curve: {pw: %f, ct: %lu, type: %d}", it->time, it->value, it->curve.coefficients.powerValue, it->curve.coefficients.cutoffTime, it->curve.type);
        }
    };
    float update();

private:
    std::vector<Keyframe> _keyframes;
    SceneMode _mode;
    LightChrono _chrono;
    float interpolate(unsigned long currentTime, unsigned long totalTime, float startValue, float endValue, Curve curve);
};

#endif