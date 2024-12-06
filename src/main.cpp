#include <MidiSerial.h>
#include <Oled.h>
#include <Flash.h>
#include <SceneBank.h>
#include <Utils.h>

MidiSerial midiSerial;
Oled screen;
RP2040Flash flash;
StateManager stateManager;

void setup()
{
    // USB Serial
    Serial.begin(115200);

    // MIDI Serial
    midiSerial.setup();

    while (!Serial && millis() < 5000)
        ;
}

void setup1()
{
    // OLED screen
    screen.setup();

    // Flash setup
    flash.begin();

    StateManager::createAndSaveStubs(flash);

    // Read from flash
    stateManager.deserialize(flash);

    // Dump scene
    stateManager.getCurrentScene()->dump();

    // Setup led bank
    stateManager.setup();

    // Done!
    debug(1, "[setup] Setup complete");
}

void loop()
{
    midiSerial.loop(stateManager);
}

void loop1()
{
    stateManager.update();

    screen.loop(stateManager);
}
