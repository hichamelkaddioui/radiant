#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <Arduino.h>
#include <Graph.h>
#include <Sequence.h>
#include <Led.h>

struct LedEffect
{
    LedEffect() = default;
    LedEffect(Led *led, Sequence *hueA, Sequence *hueB, Sequence *brightnessA, Sequence *brightnessB) : led(led), hueA(hueA), hueB(hueB), brightnessA(brightnessA), brightnessB(brightnessB) {}

    Led *led = nullptr;
    Sequence *hueA = nullptr;
    Sequence *hueB = nullptr;
    Sequence *brightnessA = nullptr;
    Sequence *brightnessB = nullptr;
};

class Scene
{
public:
    Scene() = default;
    Scene(std::vector<LedEffect> ledScenes) : _ledEffects(ledScenes) {}

    float _ab = 0.0f;
    std::vector<LedEffect> _ledEffects;

    void update();
    void restart();
    void onNotePlayed(uint8_t note, uint8_t velocity);
    size_t serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const;
    size_t deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank);

#ifdef DEBUG
    void dump();
#endif
};

class SceneBank
{
public:
    SceneBank() = default;

    int currentSceneId = 0;
    std::map<int, Scene *> _scenes;

    void next();
    void previous();
    void restart();
    void update();
    Scene *getCurrentScene() const;
    size_t serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const;
    size_t deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank);
};

#endif