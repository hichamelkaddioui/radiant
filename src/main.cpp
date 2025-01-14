#include <MidiSerial.h>
#include <Oled.h>
#include <SceneBank.h>
#include <Utils.h>

MidiSerial midiSerial;
Oled screen;
StateManager stateManager;

void setup()
{
    // USB Serial
    Serial.begin(115200);

    // MIDI Serial
    midiSerial.setup();

    // OLED screen
    screen.setup();

    while (!Serial && millis() < 5000)
        ;

    // Flash
    stateManager.setupFlash();

    // Optional: create default stubs
    // StateManager::createAndSaveStubs();

    // Read from flash
    stateManager.deserialize();

    // Dump scene
    stateManager.getCurrentScene()->dump();

    // Setup led bank
    stateManager.setupLeds();

    // Done!
    debug(1, "[setup] Setup complete");
}

void loop()
{
    midiSerial.loop(stateManager);

    stateManager.loop();

    screen.loop(stateManager);
}
