#include <Arduino.h>
#include <vector>

#ifndef KEYFRAME_H
#define KEYFRAME_H

typedef enum InterpolationMethod
{
    LINEAR,
    POWER,
    GATE,
} IM;

struct InterpolationParams
{
    float powerValue;
    unsigned long cutoffTime;

    InterpolationParams(float pw, unsigned long ct) : powerValue(pw), cutoffTime(ct) {}
};

typedef std::pair<InterpolationMethod, InterpolationParams> InterpolationMethodPair;

struct Keyframe
{
    unsigned long time;
    float value;
    std::vector<InterpolationMethodPair> methodsAndParams;

    Keyframe(unsigned long time = 0, float value = 0) : time(time), value(value) {}
};

class KeyframeList
{
public:
    Keyframe *_keyframes;
    int _count = 0;

    /**
     * Creates an InterpolationMethodPair with the given method, powerValue, and cutoffTime.
     *
     * The InterpolationMethodPair is a std::pair of InterpolationMethod and InterpolationParams.
     * The InterpolationParams are created with the given powerValue and cutoffTime.
     *
     * \param method The InterpolationMethod of the InterpolationMethodPair.
     * \param powerValue The powerValue of the InterpolationParams.
     * \param cutoffTime The cutoffTime of the InterpolationParams.
     *
     * \return The created InterpolationMethodPair.
     */
    static InterpolationMethodPair IMP(InterpolationMethod method, float powerValue = 1.0f, unsigned long cutoffTime = 0)
    {
        InterpolationParams *params = new InterpolationParams(powerValue, cutoffTime);

        return std::make_pair(method, *params);
    }
    /**
     * Creates a new Keyframe with the given time, value, and methodsAndParams.
     *
     * \param time The time of the Keyframe.
     * \param value The value of the Keyframe.
     * \param methodsAndParams The methodsAndParams of the Keyframe.
     *
     * \return The created Keyframe.
     */
    static Keyframe K(unsigned long time, float value, const std::vector<InterpolationMethodPair> &methodsAndParams = {})
    {
        Keyframe keyframe = Keyframe(time, value);
        keyframe.methodsAndParams = methodsAndParams;

        return keyframe;
    }

    KeyframeList(int count = 0, Keyframe *keyframes = nullptr) : _count(count), _keyframes(keyframes) {};
    /**
     * Updates the current value of the keyframe list based on the given current time.
     *
     * This method iterates through the list of keyframes to find the two keyframes
     * that the given current time falls between. It then calculates the
     * interpolation value between the two values and returns it.
     *
     * If the given current time is outside of the range of the keyframes, the
     * method will return the value of the last keyframe.
     *
     * \param currentTime The current time to update against.
     * \return The interpolated value.
     */
    float update(unsigned long currentTime);
    /**
     * Destroys the keyframe list and frees the memory allocated for the keyframes.
     *
     * The caller of this method is responsible for ensuring that the keyframe list
     * is not in use by any other parts of the program before calling this method.
     */
    void destroy();

private:
    /**
     * Interpolates between start and end values using a sequence of interpolation methodsAndParams.
     * Each interpolation method is applied in order, allowing for complex easing curves.
     * The result is clamped to the range [start, end].
     *
     * \param start The value to start at.
     * \param end The value to end at.
     * \param t The value to interpolate between start and end, in the range [0, 1].
     * \param methodsAndParams The sequence of interpolation methodsAndParams to apply.
     * \return The interpolated value.
     */
    float interpolate(float start, float end, float t, const std::vector<InterpolationMethodPair> &methodsAndParams);
};

#endif