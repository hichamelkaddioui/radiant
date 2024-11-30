#ifndef SCENE_H
#define SCENE_H

#include <Arduino.h>
#include <vector>
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
    std::vector<LedEffect> _ledEffects;

    void update();
    void restart();
    void onNotePlayed(uint8_t note, uint8_t velocity);
    void dump();
};

class SceneBank
{
public:
    SceneBank() = default;

    int currentScene = 0;
    std::vector<Scene *> _scenes;

    void next();
    void previous();
    void restart();
    void update();
    Scene *getCurrentScene();
    size_t serialize(uint8_t *buffer, LedBank *ledBank, GraphBank *graphBank);
    size_t deserialize(const uint8_t *buffer);
};

#endif