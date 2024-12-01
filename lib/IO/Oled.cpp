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
void Oled::handleButtonPress(SceneBank &sceneBank)
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
                debug(1, "[oled] Button A pressed");

                sceneBank.next();
            }
        }
    }

    _buttonA->lastState = reading;
}

void Oled::loop(SceneBank &sceneBank)
{
    handleButtonPress(sceneBank);
    displaySceneData(sceneBank);
}

void Oled::displaySceneData(SceneBank &sceneBank)
{
    if (millis() % 10 != 0)
        return;

    Scene *scenePtr = sceneBank.getCurrentScene();

    if (scenePtr == nullptr)
        return;

    Scene &scene = *scenePtr;

    int x2, elapsed;
    int maxCols = min(3, scene._ledEffects.size());
    int xOffset = SCREEN_WIDTH / maxCols;
    int xMax = xOffset - 5;

    Sequence *hueA;
    Sequence *brightnessA;
    Sequence *hueB;
    Sequence *brightnessB;

    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.print("Scene ");
    _display.print(sceneBank.currentScene + 1);

    int i = 0;

    for (LedEffect ledEffect : scene._ledEffects)
    {
        if (maxCols <= i)
            continue;

        int offset = i * xOffset;

        hueA = ledEffect.hueA;
        brightnessA = ledEffect.brightnessA;
        hueB = ledEffect.hueB;
        brightnessB = ledEffect.brightnessB;

        x2 = min(xMax, xMax * hueA->elapsed() / hueA->_duration);
        _display.drawLine(offset, 10, offset + x2, 10, SSD1306_WHITE);
        x2 = min(xMax, xMax * hueB->elapsed() / hueB->_duration);
        _display.drawLine(offset, 15, offset + x2, 15, SSD1306_WHITE);
        x2 = min(xMax, xMax * brightnessA->elapsed() / brightnessA->_duration);
        _display.drawLine(offset, 20, offset + x2, 20, SSD1306_WHITE);
        x2 = min(xMax, xMax * brightnessB->elapsed() / brightnessB->_duration);
        _display.drawLine(offset, 25, offset + x2, 25, SSD1306_WHITE);

        i++;
    }

    _display.display();
}