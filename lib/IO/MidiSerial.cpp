#include <Utils.h>
#include <MidiSerial.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART);

void MidiSerial::setup()
{
    // MIDI Serial
    Serial1.begin(31250);

    MidiUART.begin(MIDI_CHANNEL_OMNI);
}

void MidiSerial::loop(Scene *currentScene)
{
    // Read incoming MIDI messages
    while (MidiUART.read())
    {
        // Handle the MIDI message
        byte type = MidiUART.getType();

        if (type == MidiType::NoteOn)
        {
            byte channel = MidiUART.getChannel();
            byte note = MidiUART.getData1();
            byte velocity = MidiUART.getData2();
            debug(1, "[midi] Received note on channel %02X: %02X %02X %02X", channel, type, note, velocity);
            currentScene->onNotePlayed(note, velocity);
        }
        else if (type == MidiType::SystemExclusive)
        {
            byte channel = MidiUART.getChannel();
            const byte *array = MidiUART.getSysExArray();
            unsigned size = MidiUART.getSysExArrayLength();

            Serial.print("[midi] System Exclusive: ");

            for (int i = 0; i < size; i++)
            {
                Serial.print(array[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
        else if (type == MidiType::ControlChange)
        {
            byte channel = MidiUART.getChannel();
            byte control = MidiUART.getData1();
            byte value = MidiUART.getData2();
            debug(1, "[midi] Received control change: %02X %02X %02X", type, control, value);
        }
    }
}