#include <Arduino.h>

const int fbESCPin = 14;
const int fwESCPin = 25;
const int bbESCPin = 26;
const int bwESCPin = 27;

const int channel = 0;
const int freq = 50;      // 50 Hz
const int resolution = 16;

void writeESC(int us) {
    uint32_t duty = (uint32_t)((float)us / 20000.0 * ((1 << resolution) - 1));
    ledcWrite(channel, duty);
}

void setup() {
    Serial.begin(115200);
    Serial.print("Test");
    ledcSetup(channel, freq, resolution);
    ledcAttachPin(fbESCPin, channel);
    ledcAttachPin(fwESCPin, channel);
    ledcAttachPin(bbESCPin, channel);
    ledcAttachPin(bwESCPin, channel);

    // Send minimum throttle
    writeESC(1000);

    // Allow ESC to arm
    delay(5000);
}

void loop() {
    writeESC(1200);
    delay(2000);

    writeESC(1500);
    delay(2000);

    writeESC(1000);
    delay(2000);
}