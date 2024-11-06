#include <Oled.h>
#include <MidiSerial.h>
#include <Pixel.h>
#include <Graph.h>

MidiSerial midiSerial;
Oled screen;
Pixel pixel;
GraphBank gb = defaultGraphBank();

void setup()
{
    // USB Serial
    Serial.begin(115200);

    // MIDI Serial
    midiSerial.setup();
}

void setup1()
{
    // OLED screen
    screen.setup();

    pixel._hue = Sequence(0, 255, 10000, 1, PlaybackMode::REPEAT);
    pixel._brightness = Sequence(0, 200, 3000, 3, PlaybackMode::ONCE);
    pixel.setup();
}

void loop()
{
    midiSerial.loop();
}

void loop1()
{
    pixel.loop(gb);
    screen.loop();
}
