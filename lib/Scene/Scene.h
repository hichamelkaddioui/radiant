#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <Arduino.h>
#include <Sequence.h>
#include <GraphBank.h>
#include <LedBank.h>

struct LedEffect
{
    LedEffect() = default;
    LedEffect(Led *led, Sequence *hueA, Sequence *hueB, Sequence *brightnessA, Sequence *brightnessB) : led(led), hueA(hueA), hueB(hueB), brightnessA(brightnessA), brightnessB(brightnessB) {}

    Led *led = nullptr;
    Sequence *hueA = nullptr;
    Sequence *hueB = nullptr;
    Sequence *brightnessA = nullptr;
    Sequence *brightnessB = nullptr;

    void dump(int ledId) const;
};

class Scene
{
public:
    Scene() = default;
    Scene(std::map<int, LedEffect> ledScenes) : _ledEffects(ledScenes) {}

    float _ab = 0.0f;
    std::map<int, LedEffect> _ledEffects;

    void update();
    void restart();
    void onNotePlayed(uint8_t note, uint8_t velocity);
    size_t serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const;
    size_t deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank);
    void sysExSetHueBrightness(int messageId, int lightId, Led *led, Sequence *sequence);

    void dump();
};

#endif