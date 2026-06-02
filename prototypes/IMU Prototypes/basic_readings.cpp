// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Tracks angle of the drone
float pitchAngle = 0.0;
unsigned long lastTime = 0; // milliseconds

// PID State
float errorSum = 0.0; // I
float lastError = 0.0; // D

// PID Tuning Knobs (Work on tuning Kp first on breadboard)
float Kp = 1.2; // P 
float Ki = 0.0; // I 
float Kd = 0.0; // D

// Target Angle (for the setpoint)
float targetPitch = 0.0; 

// computePID function
float computePID(float setpoint, float measured, float dt) {
  float error = setpoint - measured;          // negative = tilted one way, positive = other

  // Integral — accumulates error over time
  errorSum += error * dt;
  // Anti-windup: clamp so integral doesn't grow forever during a crash/flip
  // constrain(x, low, high) is equivalent to Python's max(low, min(high, x))
  errorSum = constrain(errorSum, -200.0, 200.0);

  // Derivative — rate of change of error (how fast are we improving or worsening?)
  float dError = (error - lastError) / dt;
  lastError = error;                           // save for next call

  return (Kp * error) + (Ki * errorSum) + (Kd * dError);
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  lastTime = millis(); // seed the timer for PID
  delay(100);
}

void loop() {

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // /* Print out the values */
  // Serial.print("Acceleration X: ");
  // Serial.print(a.acceleration.x);
  // Serial.print(", Y: ");
  // Serial.print(a.acceleration.y);
  // Serial.print(", Z: ");
  // Serial.print(a.acceleration.z);
  // Serial.println(" m/s^2");

  // Serial.print("Rotation X: ");
  // Serial.print(g.gyro.x);
  // Serial.print(", Y: ");
  // Serial.print(g.gyro.y);
  // Serial.print(", Z: ");
  // Serial.print(g.gyro.z);
  // Serial.println(" rad/s");

  // Serial.print("Temperature: ");
  // Serial.print(temp.temperature);
  // Serial.println(" degC");

  // Serial.println("");

   // ── 2. Compute dt (time since last loop, in seconds) ────────────────────────
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;   // convert ms → seconds
  lastTime = now;

  // Guard: skip this iteration if dt is zero or implausibly large
  // (can happen on the very first loop or after a serial stall)
  if (dt <= 0.0 || dt > 0.5) return;

  // ── 3. Complementary filter — fuse gyro + accel into a stable angle ─────────
  //
  // Accelerometer pitch: atan2 gives angle from gravity vector.
  // This is accurate when still, noisy when vibrating.
  float accelPitch = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  //                                                               ^^^^^^^^^^
  //                                         converts radians → degrees

  // Gyro rate is in rad/s — convert to deg/s, then integrate over dt
  float gyroPitchRate = g.gyro.x * 180.0 / PI;   // rad/s → deg/s

  // Blend: 98% trust gyro short-term, 2% trust accel long-term
  pitchAngle = 0.98 * (pitchAngle + gyroPitchRate * dt) + 0.02 * accelPitch;

  // ── 4. Run PID ──────────────────────────────────────────────────────────────
  float pidOutput = computePID(targetPitch, pitchAngle, dt);

  // PID DEBUG OUTPUT
  Serial.print("Pitch: "); Serial.print(pitchAngle, 2);
  Serial.print("  Error: "); Serial.print(targetPitch - pitchAngle, 2);
  Serial.print("  PID: ");   Serial.println(pidOutput, 2);

  delay(5);

}