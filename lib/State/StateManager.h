#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Flash.h>
#include <SceneBank.h>

const int FLASH_BUFFER_SIZE = 1024 * 4;

enum SysExMessage
{
    SET_PARAMS = 1,
    CREATE_LIGHT = 2,
    CREATE_SCENE = 3,
    CREATE_GRAPH = 4,
    SET_HUE_A = 5,
    SET_BRIGHTNESS_A = 6,
    SET_HUE_B = 7,
    SET_BRIGHTNESS_B = 8,
    SET_STROBE_A = 9,
    SET_STROBE_B = 10,
};

class StateManager
{
public:
    StateManager(){};
    StateManager(LedBank &ledBank, GraphBank &graphBank, SceneBank &sceneBank) : lb(ledBank), gb(graphBank), sb(sceneBank){};

    void setup();
    void update();

    // Scene
    Scene *getCurrentScene();
    int getCurrentSceneId();

    // MIDI
    void handleProgramChange(byte value);
    void handleControlChange(byte type, byte value);
    void handleNoteOn(byte note, byte velocity);
    void handleSystemExclusive(const byte *buffer, unsigned long length);

    // OLED
    void handleOledButtonPress();

    // Serialization
    void serialize(RP2040Flash flash);
    void deserialize(RP2040Flash flash);
    static void createAndSaveStubs(RP2040Flash flash);

private:
    GraphBank gb;
    LedBank lb;
    SceneBank sb;
};

#endif