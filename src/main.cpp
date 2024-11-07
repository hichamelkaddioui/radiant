#include <vector>
#include <MidiSerial.h>
#include <Pixel.h>
#include <Graph.h>
#include <Utils.h>

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

    // Cycle through all hues, re-trigger on note 60
    pixel._hue = Sequence(DefaultGraph::GATE, UTILS_HUE_BLUE, UTILS_HUE_GOLDEN_YELLOW, 10000, PlaybackMode::REPEAT, 1 / 2.0f, 0x3C);
    //  Log brightness decay from max to min, re-trigger on note 61
    pixel._brightness = Sequence(DefaultGraph::GATE, 100, 255, 500, PlaybackMode::ONCE, 1 / 5.0f, 0x3D);
    pixel.setup();

    for (int i = 0; i < pixels.size(); i++)
    {
        pixels[i]->_hue.restart();
        pixels[i]->_brightness.restart();
        pixels[i]->dump();
    }

    debug(1, "[setup] Setup complete");
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
