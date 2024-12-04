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

    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.print("Scene ");
    _display.print(sceneBank.currentSceneId);

    int xA = SCREEN_WIDTH / 2;
    int xB = SCREEN_WIDTH - 6;

    _display.setCursor(xA, 0);
    _display.print("A");
    _display.setCursor(xB, 0);
    _display.print("B");

    int rectangleStart = xA + 8;
    int rectangleWidth = xB - (xA + 8) - 3;

    _display.drawRoundRect(xA + 8, 0, xB - (xA + 8) - 3, 7, 3, SSD1306_WHITE);

    int middleRectangle = rectangleStart + rectangleWidth / 2;
    int halfLengthRectangle = rectangleWidth / 2;
    int width = halfLengthRectangle * abs((map(scene._ab * 1000, 0, 1000, -1000, 1000) / 1000.0f)) - 2;

    if (scene._ab < 0.5)
    {
        _display.fillRoundRect(middleRectangle - width, 2, width, 3, 3, SSD1306_WHITE);
    }
    else
    {
        _display.fillRoundRect(middleRectangle, 2, width, 3, 3, SSD1306_WHITE);
    }

    int maxCols = min(3, scene._ledEffects.size());
    int xOffset = SCREEN_WIDTH / maxCols;
    int maxHeight = SCREEN_HEIGHT - 15;
    int gap = min(5, xOffset / 5);
    int i = 0;

    for (LedEffect ledEffect : scene._ledEffects)
    {
        if (maxCols <= i)
            continue;

        int offset = i * xOffset;

        Sequence *hueA = ledEffect.hueA;
        Sequence *brightnessA = ledEffect.brightnessA;
        Sequence *hueB = ledEffect.hueB;
        Sequence *brightnessB = ledEffect.brightnessB;

        int yHueA = min(maxHeight, maxHeight * hueA->elapsed() / hueA->_duration);
        int yHueB = min(maxHeight, maxHeight * hueB->elapsed() / hueB->_duration);
        int yBrightnessA = min(maxHeight, maxHeight * brightnessA->elapsed() / brightnessA->_duration);
        int yBrightnessB = min(maxHeight, maxHeight * brightnessB->elapsed() / brightnessB->_duration);

        _display.setCursor(offset, SCREEN_HEIGHT / 2 + 3);
        _display.print(i + 1);
        _display.drawLine(offset + 2 * gap, SCREEN_HEIGHT, offset + 2 * gap, SCREEN_HEIGHT - yHueA, SSD1306_WHITE);
        _display.drawLine(offset + 3 * gap, SCREEN_HEIGHT, offset + 3 * gap, SCREEN_HEIGHT - yHueB, SSD1306_WHITE);
        _display.drawLine(offset + 5 * gap, SCREEN_HEIGHT, offset + 5 * gap, SCREEN_HEIGHT - yBrightnessA, SSD1306_WHITE);
        _display.drawLine(offset + 6 * gap, SCREEN_HEIGHT, offset + 6 * gap, SCREEN_HEIGHT - yBrightnessB, SSD1306_WHITE);

        i++;
    }

    _display.display();
}