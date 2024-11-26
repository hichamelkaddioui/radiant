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
    delay(1000);

    for (size_t i = 0; i < 15; i++)
    {
        _display.invertDisplay(true);
        _display.display();
        delay(50);
        _display.invertDisplay(false);
        _display.display();
        delay(50);
    }

    delay(950);
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

    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);

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

void Oled::displaySceneData(const Scene &scene)
{
    if (millis() % 10 != 0)
        return;

    LedEffect first = scene._ledEffects[0];
    Sequence *hue = first.hueA;
    Sequence *brightness = first.brightnessA;

    int x2, elapsed;
    int xMax = SCREEN_WIDTH / 2 - 5;
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.print("Hue");
    x2 = min(xMax, xMax * hue->elapsed() / hue->_graphOptions.duration);
    _display.drawLine(0, 18, x2, 18, SSD1306_WHITE);
    int xOffset = SCREEN_WIDTH / 2;
    _display.setCursor(xOffset, 0);
    _display.print("Brightness");
    x2 = min(xOffset + xMax, xOffset + (xMax * brightness->elapsed() / brightness->_graphOptions.duration));
    _display.drawLine(xOffset, 18, x2, 18, SSD1306_WHITE);
    _display.display();
}