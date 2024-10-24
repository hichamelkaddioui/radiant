#include <Arduino.h>
#include <Scene.h>
#include <Utils.h>

void flashThreeTimes(int delayTime = 100)
{
    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(delayTime);
        digitalWrite(LED_BUILTIN, LOW);
        delay(delayTime);
    }
}

Scene scene;

void setup()
{
    Serial.begin(9600);

    while (!Serial)
        ;

    flashThreeTimes();

    // Create the scene
    std::vector<Keyframe> keyframes;

    keyframes.push_back(Keyframe(0, 0.0f, Curve(CurveType::EASE, CurveCoefficient(2.0f))));
    keyframes.push_back(Keyframe(100, 10.0f, Curve(CurveType::WAVE, CurveCoefficient(120))));
    keyframes.push_back(Keyframe(1000, -10.0f, Curve(CurveType::LINEAR, CurveCoefficient(-1))));
    keyframes.push_back(Keyframe(1200, 0.0f, Curve(CurveType::EASE, CurveCoefficient(1 / 2.0f))));

    scene = Scene(keyframes, SceneMode::LOOP);

    debug(1, "x,y");
}

void loop()
{
    float value = scene.update();
    debug(7, "%f,%f", float(millis()) / 1000, value);
}
