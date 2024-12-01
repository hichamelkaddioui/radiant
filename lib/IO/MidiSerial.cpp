#include <Utils.h>
#include <MidiSerial.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART);

void MidiSerial::setup()
{
    // MIDI Serial
    Serial1.begin(31250);

    MidiUART.begin(MIDI_CHANNEL_OMNI);
}

void MidiSerial::handleNoteOn(SceneBank &sceneBank)
{
    byte note = MidiUART.getData1();
    byte velocity = MidiUART.getData2();
    debug(1, "[midi] received note on: %02X, velocity: %02X", note, velocity);

    Scene *currentScene = sceneBank.getCurrentScene();

    if (!currentScene)
    {
        debug(1, "[midi] no current scene");
        return;
    }

    currentScene->onNotePlayed(note, velocity);
}

void MidiSerial::handleSystemExclusive(SceneBank &sceneBank)
{
    const byte *array = MidiUART.getSysExArray();
    unsigned size = MidiUART.getSysExArrayLength();

    Serial.print("[midi] received System Exclusive: ");

    for (int i = 0; i < size; i++)
    {
        Serial.print(array[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void MidiSerial::handleProgramChange(SceneBank &sceneBank)
{
    byte value = MidiUART.getData1();
    debug(1, "[midi] received program change: %02X", value);

    switch (value)
    {
    case 0x00:
        sceneBank.restart();
        break;
    case 0x01:
        sceneBank.next();
        break;
    default:
        break;
    }
}

void MidiSerial::handleControlChange(SceneBank &sceneBank)
{
    byte type = MidiUART.getData1();
    byte value = MidiUART.getData2();
    debug(1, "[midi] received control change: %02X %02X", type, value);

    switch (type)
    {
    // Balance
    case 0x08:
        sceneBank.getCurrentScene()->_ab = value / 127.0f;
        break;
    default:
        break;
    }
}

void MidiSerial::loop(SceneBank &sceneBank)
{
    // Read incoming MIDI messages
    while (MidiUART.read())
    {
        // Handle the MIDI message
        byte type = MidiUART.getType();

        switch (type)
        {
        case MidiType::NoteOn:
            handleNoteOn(sceneBank);
            break;
        case MidiType::SystemExclusive:
            handleSystemExclusive(sceneBank);
            break;
        case MidiType::ProgramChange:
            handleProgramChange(sceneBank);
            break;
        case MidiType::ControlChange:
            handleControlChange(sceneBank);
            break;
        default:
            debug(1, "[midi] received midi message type: %02X, value: %02X", type, MidiUART.getData1());
            break;
        }
    }
}