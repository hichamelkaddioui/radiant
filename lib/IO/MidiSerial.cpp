#include <MidiSerial.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART);

void MidiSerial::setup()
{
    // MIDI Serial
    Serial1.begin(31250);

    MidiUART.begin(MIDI_CHANNEL_OMNI);
}

void MidiSerial::loop(std::vector<Pixel *> pixels)
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
            for (int i = 0; i < pixels.size(); i++)
            {
                pixels[i]->onNotePlayed(note, velocity);
            }
        }

        Serial.print("Received: ");
        Serial.print(type, HEX);
        Serial.print(" ");
        Serial.print(channel, HEX);
        Serial.print(" ");
        Serial.print(note, HEX);
        Serial.print(" ");
        Serial.print(velocity);
        Serial.print(" ");
        Serial.println();
    }
}