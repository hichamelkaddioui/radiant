#include <Utils.h>
#include <MidiSerial.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART);

void MidiSerial::setup()
{
    // MIDI Serial
    Serial1.begin(31250);

    MidiUART.begin(MIDI_CHANNEL_OMNI);
}

void MidiSerial::loop(Scene *scene)
{
    // Read incoming MIDI messages
    while (MidiUART.read())
    {
        // Handle the MIDI message
        byte type = MidiUART.getType();
        byte channel = MidiUART.getChannel();
        byte note = MidiUART.getData1();
        byte velocity = MidiUART.getData2();

        if (type == 0x90)
        {
            scene->onNotePlayed(note, velocity);
        }

        debug(1, "[midi] Received MIDI message: %02X %02X %02X %02X", type, channel, note, velocity);
    }
}