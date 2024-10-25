#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <LightChrono.h>
#include <Utils.h>

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
    Scene(std::vector<Keyframe> keyframes = {}, SceneMode mode = SceneMode::LOOP) : _keyframes(keyframes), _mode(mode){};
    float update();
    void trigger();
    void addKeyframes(std::vector<Keyframe> keyframes);
    static std::vector<Keyframe> impulses(unsigned long startTime, float power, int numImpulses)
    {
        std::vector<Keyframe> keyframes;

        int step = 100;

        for (unsigned long i = 0; i < numImpulses; i++)
        {
            keyframes.push_back(Keyframe(startTime + i * step, 0, Curve::ease(power)));
            keyframes.push_back(Keyframe(startTime + (i + 1) * step - 1, 255, Curve::linear()));
        }

        return keyframes;
    };

    size_t serialize(uint8_t *buffer) const
    {
        size_t offset = 0;

        // Serialize mode
        memcpy(buffer + offset, &_mode, sizeof(SceneMode));
        offset += sizeof(SceneMode);

        // Serialize number of keyframes
        size_t keyframeCount = _keyframes.size();
        memcpy(buffer + offset, &keyframeCount, sizeof(size_t));
        offset += sizeof(size_t);

        // Serialize each keyframe
        for (const Keyframe &keyframe : _keyframes)
        {
            memcpy(buffer + offset, &keyframe.time, sizeof(unsigned long));
            offset += sizeof(unsigned long);
            memcpy(buffer + offset, &keyframe.value, sizeof(float));
            offset += sizeof(float);
            memcpy(buffer + offset, &keyframe.curve.type, sizeof(CurveType));
            offset += sizeof(CurveType);

            // Serialize curve coefficients based on the type
            size_t coefficientsLength = 0;
            switch (keyframe.curve.type)
            {
            case EASE:
                coefficientsLength = sizeof(float);
                memcpy(buffer + offset, &keyframe.curve.coefficient.powerValue, coefficientsLength);
                break;
            case WAVE:
            case GATE:
                coefficientsLength = sizeof(float) * 2 + sizeof(unsigned long);
                memcpy(buffer + offset, &keyframe.curve.coefficient.rangeAndPeriod, coefficientsLength);
                break;
            case LINEAR:
                // No additional data for linear
                break;
            }
            offset += coefficientsLength;
        }

        // Return the total size of the serialized data
        return offset;
    }

    void deserialize(const uint8_t *buffer, size_t length)
    {
        size_t offset = 0;

        // Deserialize mode
        memcpy(&_mode, buffer + offset, sizeof(SceneMode));
        offset += sizeof(SceneMode);

        // Deserialize number of keyframes
        size_t keyframeCount;
        memcpy(&keyframeCount, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        _keyframes.clear();                // Clear existing keyframes
        _keyframes.reserve(keyframeCount); // Reserve space for keyframes

        // Deserialize each keyframe
        for (size_t i = 0; i < keyframeCount; ++i)
        {
            Keyframe keyframe;
            memcpy(&keyframe.time, buffer + offset, sizeof(unsigned long));
            offset += sizeof(unsigned long);
            memcpy(&keyframe.value, buffer + offset, sizeof(float));
            offset += sizeof(float);
            memcpy(&keyframe.curve.type, buffer + offset, sizeof(CurveType));
            offset += sizeof(CurveType);

            // Deserialize curve coefficients based on the type
            switch (keyframe.curve.type)
            {
            case EASE:
                memcpy(&keyframe.curve.coefficient.powerValue, buffer + offset, sizeof(float));
                offset += sizeof(float);
                break;
            case WAVE:
            case GATE:
            {
                float min, max;
                unsigned long period;
                memcpy(&min, buffer + offset, sizeof(float));
                offset += sizeof(float);
                memcpy(&max, buffer + offset, sizeof(float));
                offset += sizeof(float);
                memcpy(&period, buffer + offset, sizeof(unsigned long));
                offset += sizeof(unsigned long);
                keyframe.curve.coefficient = CurveCoefficient(min, max, period);
                break;
            }
            case LINEAR:
                // No additional data for linear
                break;
            }
            _keyframes.push_back(keyframe);
        }
    }

    void dump() const
    {
#ifdef DEBUG
        debug(1, "[scene] mode: %s", _mode == SceneMode::LOOP ? "LOOP" : "TRIGGER");
        int i = 0;
        for (const Keyframe &keyframe : _keyframes)
        {
            if (keyframe.curve.type == CurveType::EASE)
                debug(1, "[scene]\tkeyframe[%d]: %5lu\t%12f\t%s\tpw=%f", i, keyframe.time, keyframe.value, "ease", keyframe.curve.coefficient.powerValue);
            if (keyframe.curve.type == CurveType::WAVE)
                debug(1, "[scene]\tkeyframe[%d]: %5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i, keyframe.time, keyframe.value, "wave", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
            if (keyframe.curve.type == CurveType::GATE)
                debug(1, "[scene]\tkeyframe[%d]: %5lu\t%12f\t%s\tmin=%f\tmax=%f\tperiod=%lu", i, keyframe.time, keyframe.value, "gate", keyframe.curve.coefficient.rangeAndPeriod.min, keyframe.curve.coefficient.rangeAndPeriod.max, keyframe.curve.coefficient.rangeAndPeriod.period);
            if (keyframe.curve.type == CurveType::LINEAR)
                debug(1, "[scene]\tkeyframe[%d]: %5lu\t%12f\t%s", i, keyframe.time, keyframe.value, "linear");

            i++;
        }
#endif
    }

private:
    std::vector<Keyframe> _keyframes;
    SceneMode _mode;
    LightChrono _chrono;
    float interpolate(unsigned long currentTime, unsigned long totalTime, float startValue, float endValue, Curve curve);
};

#endif