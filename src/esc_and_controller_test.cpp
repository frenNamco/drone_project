#include <Arduino.h>
#include <Bluepad32.h>

#define ESC_PIN     18
#define CHANNEL     0
#define FREQ        50
#define RESOLUTION  16

ControllerPtr controllers[BP32_MAX_GAMEPADS]; 

int currentMillis = 0;
int previousMillis = 0;
int timeStep = 50;

void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            controllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (controllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            controllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void writeESC(int us) {
    // Converts microseconds to duty cycle for a 50 Hz, 16 bit resolution PWM signal
    uint32_t duty = (uint32_t)((float)us / 20000.0 * ((1 << RESOLUTION) - 1));
    ledcWrite(CHANNEL, duty);
}

void setup() {
    Serial.begin(115200);
    Serial.print("Controller ESC Test");

    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    BP32.enableVirtualDevice(false);

    ledcSetup(CHANNEL, FREQ, RESOLUTION);
    ledcAttachPin(ESC_PIN, CHANNEL);
    writeESC(1000);
    delay(5000);

    previousMillis = millis();
}

int controllerValue;                    // Value from pressing the left trigger on DS4
int currentThrottlePercent;             // Current throttle percent of motors, from 0% to 100%
int currentThrottlePercentChange;       // Current change to be induced on throttle
int previousThrottlePercentChange;      // Previous change induced on throttle
int us;                                 // Duty cycle for PWM signal sent to ESC in microseconds

int maxThrottlePercentChange = 5;


void loop() {
   bool dataUpdated = BP32.update();

   currentMillis = millis();
   Serial.print(currentMillis - previousMillis);
   Serial.print(",");
   if ((currentMillis - previousMillis) > timeStep) {
       previousMillis = currentMillis;

       if (currentThrottlePercentChange != previousThrottlePercentChange) {
           previousThrottlePercentChange = currentThrottlePercentChange;
       }
   }

   if (dataUpdated) {
        Serial.print(controllers[0]->throttle());
        Serial.print(",");

        controllerValue = controllers[0]->throttle(); 
        currentThrottlePercentChange = map(controllerValue, 0, 1023, 0, maxThrottlePercentChange);
        
        if (currentThrottlePercent >= 0 && currentThrottlePercent < 100) {
            currentThrottlePercent += currentThrottlePercentChange;
        } else if (currentThrottlePercent >= 100) {
            currentThrottlePercent = 100;
        } else if (currentThrottlePercent < 0) {
            currentThrottlePercent = 0;
        }

    }
    
    if (currentThrottlePercentChange < previousThrottlePercentChange) {
        if (currentThrottlePercent <= 0) {
            currentThrottlePercent = 0;
        } else {
            currentThrottlePercent -= currentThrottlePercentChange;
        }
        
    } else if (currentThrottlePercentChange == 0) {
        if (currentThrottlePercent <= 0) {
            currentThrottlePercent = 0;
        } else {
            currentThrottlePercent -= maxThrottlePercentChange;
        }
    }


    Serial.print(currentThrottlePercentChange);
    Serial.print(",");
    Serial.print(previousThrottlePercentChange);
    Serial.print(",");
    Serial.print(currentThrottlePercent);
    Serial.print(",");
    us =  map(currentThrottlePercent, 0, 100, 1000, 2000);
    Serial.println(us);
    writeESC(us);
}