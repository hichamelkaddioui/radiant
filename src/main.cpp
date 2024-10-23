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

    keyframes.push_back(Keyframe(0, 0, Curve(CurveType::EASE, CurveCoefficients(2.0f, 1000))));
    keyframes.push_back(Keyframe(100, 500, Curve(CurveType::EASE, CurveCoefficients(1 / 2.0f, 1000))));
    keyframes.push_back(Keyframe(200, 0, Curve(CurveType::GATE, CurveCoefficients(2.0f, 5))));
    keyframes.push_back(Keyframe(500, 1000, Curve(CurveType::GATE, CurveCoefficients(2.0f, 12))));

    scene = Scene(keyframes, SceneMode::LOOP);

    debug(1, "x,y");
}

void loop()
{
    float value = scene.update();
    debug(3, "%f,%f", float(millis()) / 1000, value);
}
