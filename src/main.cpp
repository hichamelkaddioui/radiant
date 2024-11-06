#include <vector>
#include <MidiSerial.h>
#include <Pixel.h>
#include <Graph.h>

MidiSerial midiSerial;
Oled screen;
Pixel pixel;
GraphBank gb = defaultGraphBank();
std::vector<Pixel *> pixels = {&pixel};

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

    // Cycle once through all hues, re-trigger on note 60
    pixel._hue = Sequence(0, 255, 10000, DefaultGraph::UP, PlaybackMode::REPEAT, true, 0x3C);
    //  Log brightness decay from max to min, re-trigger on note 61
    pixel._brightness = Sequence(0, 255, 300, DefaultGraph::DOWN, PlaybackMode::ONCE, true, 0x3D);
    pixel.setup();

    for (int i = 0; i < pixels.size(); i++)
    {
        pixels[i]->_hue.restart();
        pixels[i]->_brightness.restart();
    }

    Serial.println("Setup done");
}

void loop()
{
    midiSerial.loop(pixels);
}

void loop1()
{
    for (int i = 0; i < pixels.size(); i++)
    {
        pixels[i]->loop(gb);

        screen.displayPixelData(*pixels[i]);
    }

    screen.loop();
}
