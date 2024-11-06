#include <Arduino.h>
#include <Utils.h>
#include <Oled.h>

unsigned long debounceDelay = 50;

Oled::Oled()
{
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    _display = display;

    _buttonA = new ButtonState(SCREEN_BUTTON_A_PIN);
    _buttonB = new ButtonState(SCREEN_BUTTON_B_PIN);
    _buttonC = new ButtonState(SCREEN_BUTTON_C_PIN);
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
    // OLED button B as a 100k pullup on it on 128x32 FW
    pinMode(_buttonB->pin, INPUT);
    pinMode(_buttonC->pin, INPUT_PULLUP);

    showGreetings();
}

/**
 * TODO: fix contact failure for pin 6 (button B) & 5 (button C)
 */
void Oled::handleButtonPress()
{
    ButtonState *buttonStates[] = {_buttonA, _buttonB, _buttonC};

    for (ButtonState *buttonState : buttonStates)
    {
        int reading = digitalRead(buttonState->pin);

        if (reading != buttonState->lastState)
        {
            buttonState->lastDebounceTime = millis();

            debug(1, "Button %d changed from %d to %d", buttonState->pin, buttonState->lastState, reading);
        }

        if ((millis() - buttonState->lastDebounceTime) > debounceDelay)
        {
            if (reading != buttonState->state)
            {
                buttonState->state = reading;
                if (buttonState->state == LOW)
                {
                    Serial.print(F("Button "));
                    Serial.print(buttonState->pin);
                    Serial.println(F(" pressed"));
                }
            }
        }

        buttonState->lastState = reading;
    }
}

void Oled::loop()
{
    handleButtonPress();
}
