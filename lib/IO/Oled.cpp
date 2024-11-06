#include <Arduino.h>
#include <Utils.h>
#include <Oled.h>

unsigned long debounceDelay = 50;

Oled::Oled()
{
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    _display = display;

    _buttonA = new ButtonState(SCREEN_BUTTON_A_PIN);
}

void Oled::showGreetings()
{
    // Show splash
    int xCenter = (SCREEN_WIDTH - splashWidth) / 2;
    int yCenter = (SCREEN_HEIGHT - splashHeight) / 2;

    _display.clearDisplay();
    _display.drawBitmap(xCenter, yCenter, splashScreenData, splashWidth, splashHeight, 1);
    _display.display();

    delay(3000);

    // Show greeting
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.printf("MOC MIDI LED driver");
    _display.setCursor(0, 16);
    _display.printf("Waiting for MIDI\r\ndata...");
    _display.display();
}

void Oled::setup()
{
    int tries = 10;
    while (!_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS) && --tries)
        delay(10);

    if (!tries)
    {
        Serial.println(F("OLED begin failed"));
        return;
    }

    pinMode(_buttonA->pin, INPUT_PULLUP);

    showGreetings();
}

/**
 * TODO: fix contact failure for pin 6 (button B) & 5 (button C)
 */
void Oled::handleButtonPress()
{
    int reading = digitalRead(_buttonA->pin);

    if (reading != _buttonA->lastState)
    {
        _buttonA->lastDebounceTime = millis();
    }

    if ((millis() - _buttonA->lastDebounceTime) > debounceDelay)
    {
        if (reading != _buttonA->state)
        {
            _buttonA->state = reading;
            if (_buttonA->state == LOW)
            {
                Serial.println(F("Button A pressed"));
            }
        }
    }

    _buttonA->lastState = reading;
}

void Oled::loop()
{
    handleButtonPress();
}
