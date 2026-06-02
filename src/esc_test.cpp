#include <Arduino.h>

const int escPin = 18;
const int channel = 0;
const int freq = 50;      // 50 Hz
const int resolution = 16;

void writeESC(int us)
{
    uint32_t duty = (uint32_t)((float)us / 20000.0 * ((1 << resolution) - 1));
    ledcWrite(channel, duty);
}

void setup()
{

    Serial.begin(9600);
    Serial.print("Test");
    ledcSetup(channel, freq, resolution);
    ledcAttachPin(escPin, channel);

    // Send minimum throttle
    writeESC(1000);

    // Allow ESC to arm
    delay(5000);
}

void loop()
{
    writeESC(1200);
    delay(2000);

    writeESC(1500);
    delay(2000);

    writeESC(1000);
    delay(2000);
}