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
    debug(1, "[midi] received note on: 0x%02X, velocity: 0x%02X", note, velocity);

    Scene *currentScene = sceneBank.getCurrentScene();

    if (!currentScene)
    {
        debug(1, "[midi] no current scene");
        return;
    }

    currentScene->onNotePlayed(note, velocity);
}

void MidiSerial::handleSystemExclusive(LedBank &ledBank, GraphBank &graphBank, SceneBank &sceneBank)
{
    const byte *buffer = MidiUART.getSysExArray();
    unsigned length = MidiUART.getSysExArrayLength();

    debug(1, "[midi] received System Exclusive");

    // Read message id
    byte messageId = buffer[2];

    switch (messageId)
    {
    case SysExMessage::CREATE_SCENE:
        sceneBank.sysExCreate(buffer + 3, length - 3);
        break;
    case SysExMessage::CREATE_GRAPH:
        graphBank.sysExCreate(buffer + 3, length - 3);
        break;
    case SysExMessage::SET_HUE_A:
    case SysExMessage::SET_BRIGHTNESS_A:
    case SysExMessage::SET_HUE_B:
    case SysExMessage::SET_BRIGHTNESS_B:
        sceneBank.sysExSetHueBrightness(buffer + 2, length - 2, ledBank, graphBank);
        break;
    case SysExMessage::SET_PARAMS:
    case SysExMessage::CREATE_LIGHT:
    case SysExMessage::SET_STROBE_A:
    case SysExMessage::SET_STROBE_B:
    default:
        debugByteArray(buffer, length);
        break;
    }
}

void MidiSerial::handleProgramChange(SceneBank &sceneBank)
{
    byte value = MidiUART.getData1();
    debug(1, "[midi] received program change: 0x%02X", value);

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
    debug(1, "[midi] received control change: 0x%02X 0x%02X", type, value);

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

void MidiSerial::loop(LedBank &ledBank, GraphBank &graphBank, SceneBank &sceneBank)
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
            handleSystemExclusive(ledBank, graphBank, sceneBank);
            break;
        case MidiType::ProgramChange:
            handleProgramChange(sceneBank);
            break;
        case MidiType::ControlChange:
            handleControlChange(sceneBank);
            break;
        default:
            debug(1, "[midi] received midi message type: 0x%02X, value: 0x%02X", type, MidiUART.getData1());
            break;
        }
    }
}