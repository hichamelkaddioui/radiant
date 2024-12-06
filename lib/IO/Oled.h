#ifndef IO_OLED_H
#define IO_OLED_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SceneBank.h>

// https://learn.adafruit.com/adafruit-oled-featherwing/pinouts

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for the display

// https://learn.adafruit.com/usb-midi-host-messenger/code-the-messenger

#define SCREEN_BUTTON_A_PIN 9 // Button A pin

const static int splashWidth = 120;
const static int splashHeight = 32;
/* clang-format off */
const uint8_t PROGMEM splashScreenData[] = {
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b01000000,0b00100000,0b00000000,0b00000111,0b11000000,0b00000000,0b00001111,0b11110000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b11100000,0b01110000,0b00000000,0b00001111,0b11100000,0b00000000,0b00011111,0b11111000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000001,0b11110000,0b11111000,0b00000000,0b00011111,0b11110000,0b00000000,0b00111111,0b11111100,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000001,0b11110000,0b11111000,0b00000000,0b00111111,0b11111000,0b00000000,0b00111111,0b11111100,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000011,0b11111001,0b11111100,0b00000000,0b00111111,0b11111000,0b00000000,0b01111111,0b11111000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00001111,0b11111111,0b11111111,0b10000001,0b11111111,0b11111111,0b00000011,0b11111111,0b11110000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00011111,0b11111111,0b11111111,0b11000011,0b11111111,0b11111111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b11111111,0b11111111,0b11000011,0b11111111,0b11111111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00011111,0b10001111,0b11000011,0b11110000,0b00011111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11111100,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11100000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11100000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11100000,0b00001111,0b10000111,0b11111100,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11110000,0b00011111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11111111,0b11111111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000011,0b11111111,0b11111111,0b10000111,0b11111110,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000001,0b11111111,0b11111111,0b00000011,0b11111111,0b11100000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000000,0b00111111,0b11111000,0b00000000,0b01111111,0b11111000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000000,0b00111111,0b11111000,0b00000000,0b00111111,0b11111100,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00111111,0b00001111,0b10000111,0b11000000,0b00011111,0b11110000,0b00000000,0b00111111,0b11111100,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00011110,0b00000111,0b00000011,0b10000000,0b00011111,0b11110000,0b00000000,0b00011111,0b11111000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00001100,0b00000010,0b00000001,0b00000000,0b00001111,0b11100000,0b00000000,0b00011111,0b11110000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
};
/* clang-format on */

struct ButtonState
{
    int pin;
    unsigned long lastDebounceTime;
    int state;
    int lastState;

    ButtonState(int pin) : pin(pin), lastDebounceTime(0), state(0), lastState(0) {}
};

class Oled
{
public:
    Oled();
    void setup();
    void loop(SceneBank &sceneBank);

    Adafruit_SSD1306 _display;

private:
    ButtonState *_buttonA;

    void showGreetings();
    void handleButtonPress(SceneBank &sceneBank);
    void displaySceneData(SceneBank &sceneBank);
};

#endif