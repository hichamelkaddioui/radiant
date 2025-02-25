#ifndef SCENE_BANK_H
#define SCENE_BANK_H

#include <map>
#include <Arduino.h>
#include <Scene.h>
#include <GraphBank.h>
#include <LedBank.h>

class SceneBank
{
public:
    int currentSceneId = 0;
    std::map<int, Scene *> _scenes;

    Scene *getCurrentScene() const;
    void restart();
    void at(byte velocity);
    void next();
    void update();
    size_t serialize(uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank) const;
    size_t deserialize(const uint8_t *buffer, const LedBank &ledBank, const GraphBank &graphBank);
    void sysExCreate(const uint8_t *buffer, size_t length);
    void sysExSetHueBrightness(const uint8_t *buffer, size_t length, const LedBank &ledBank, const GraphBank &graphBank);

    static SceneBank createDummy(const LedBank &ledBank, const GraphBank &graphBank);
};

#endif