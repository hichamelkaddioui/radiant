#include <Utils.h>
#include <MidiSerial.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART);

void MidiSerial::setup()
{
    // MIDI Serial
    Serial1.begin(31250);

    MidiUART.begin(MIDI_CHANNEL_OMNI);
}

void MidiSerial::loop(StateManager &manager)
{
    // Read incoming MIDI messages
    while (MidiUART.read())
    {
        // Handle the MIDI message
        byte type = MidiUART.getType();

        switch (type)
        {
        case MidiType::NoteOn:
            return manager.handleNoteOn(MidiUART.getData1(), MidiUART.getData2());
        case MidiType::SystemExclusive:
            return manager.handleSystemExclusive(MidiUART.getSysExArray(), MidiUART.getSysExArrayLength());
        case MidiType::ProgramChange:
            return manager.handleProgramChange(MidiUART.getData1());
        case MidiType::ControlChange:
            return manager.handleControlChange(MidiUART.getData1(), MidiUART.getData2());
        case MidiType::ActiveSensing:
            // Ignore active sensing events as they're flooding
            break;
        default:
            debug(1, "[midi] received midi message type: 0x%02X, value: 0x%02X", type, MidiUART.getData1());
            break;
        }
    }
}