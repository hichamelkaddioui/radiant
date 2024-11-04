// SPDX-FileCopyrightText: 2024 john park for Adafruit Industries
// SPDX-License-Identifier: MIT

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MIDI.h>

// Display settings
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for the display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MidiUART); // Serial MIDI

void displayMessage(byte type, byte channel, byte note, byte velocity)
{
    String messageType;
    switch (type)
    {
    case 0x00:
        messageType = "Invalid Type";
        break;
    case 0x80:
        messageType = "Note Off";
        break;
    case 0x90:
        messageType = "Note On";
        break;
    case 0xA0:
        messageType = "Polyphonic AfterTouch";
        break;
    case 0xB0:
        messageType = "Control Change";
        break;
    case 0xC0:
        messageType = "Program Change";
        break;
    case 0xD0:
        messageType = "Channel AfterTouch";
        break;
    case 0xE0:
        messageType = "Pitch Bend";
        break;
    case 0xF0:
        messageType = "System Exclusive";
        break;
    case 0xF1:
        messageType = "Time Code Quarter Frame";
        break;
    case 0xF2:
        messageType = "Song Position Pointer";
        break;
    case 0xF3:
        messageType = "Song Select";
        break;
    case 0xF4:
        messageType = "Undefined F4";
        break;
    case 0xF5:
        messageType = "Undefined F5";
        break;
    case 0xF6:
        messageType = "Tune Request";
        break;
    case 0xF7:
        messageType = "System Exclusive End";
        break;
    case 0xF8:
        messageType = "Timing Clock";
        break;
    case 0xF9:
        messageType = "Timing Tick";
        break;
    case 0xFA:
        messageType = "Start";
        break;
    case 0xFB:
        messageType = "Continue";
        break;
    case 0xFC:
        messageType = "Stop";
        break;
    case 0xFD:
        messageType = "Undefined FD";
        break;
    case 0xFE:
        messageType = "Active Sensing";
        break;
    case 0xFF:
        messageType = "System Reset";
        break;
    default:
        messageType = "Unknown";
        break;
    }

    // Print message to Serial
    Serial.printf("MIDI Type: %s, Ch: %u, Note: %u, Vel: %u", messageType.c_str(), channel, note, velocity);
    Serial.println();

    if (type != 0x90)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        Serial.printf("MIDI Type: %s\r\nCh: %u\r\nNote: %u\r\nVel: %u", messageType.c_str(), channel, note, velocity);
        return;
    }

    // Display message on OLED

    // Clear previous message
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0, 8);
    // Convert note to ABC notation
    const String notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const int noteIndex = note % 12;
    const String noteName = notes[noteIndex];
    int octave = (note - noteIndex) / 12 - 1;
    display.printf("%s%d", noteName.c_str(), octave);
    display.display();
}

void setup()
{
    Serial.begin(115200);
    // Standard MIDI baud rate
    Serial1.begin(31250);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Initial display setup
    Serial.println("OLED begun");

    // Display message on OLED
    display.clearDisplay();
    display.setTextWrap(false);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    String message = "MOC MIDI Controller";
    int messageLength = message.length();
    int leftLimit = messageLength * -18;

    for (int x = SCREEN_WIDTH; x > leftLimit; x -= 3)
    {
        display.clearDisplay();
        display.setCursor(x, 8);
        display.print(message.c_str());
        display.display();
        delay(1);
    }

    display.clearDisplay();
    display.print("LETSGO!");

    for (int x = 0; x < 10; x++)
    {
        display.invertDisplay(true);
        display.display();
        delay(50);
        display.invertDisplay(false);
        display.display();
        delay(50);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 8);
    display.print("Waiting for MIDI\r\ninput...");
    display.display();
}

void loop()
{

    // Read incoming MIDI messages
    while (MidiUART.read())
    {
        // Handle the MIDI message
        byte type = MidiUART.getType();
        byte channel = MidiUART.getChannel();
        byte note = MidiUART.getData1();
        byte velocity = MidiUART.getData2();

        displayMessage(type, channel, note, velocity);
    }
}
