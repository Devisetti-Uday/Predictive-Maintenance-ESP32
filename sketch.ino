#include <Wire.h>

#define MPU6050_ADDR 0x68

void setup() {
  Serial.begin(115200);

  // Start I2C
  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);       // Power Management register
  Wire.write(0x00);       // Wake up sensor
  Wire.endTransmission();

  Serial.println("MPU6050 initialized");
}

void loop() {
  int16_t ax, ay, az;

  // Start reading from accelerometer X register
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  // Read 6 bytes: X, Y, Z
  Wire.requestFrom(MPU6050_ADDR, 6);

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();

  // Convert raw values to acceleration in m/s²
  float accelX = ax / 16384.0 * 9.81;
  float accelY = ay / 16384.0 * 9.81;
  float accelZ = az / 16384.0 * 9.81;

  // Calculate total acceleration magnitude
  float magnitude = sqrt(
    accelX * accelX +
    accelY * accelY +
    accelZ * accelZ
  );

  Serial.print("X: ");
  Serial.print(accelX, 2);

  Serial.print(" m/s² | Y: ");
  Serial.print(accelY, 2);

  Serial.print(" m/s² | Z: ");
  Serial.print(accelZ, 2);

  Serial.print(" m/s² | Magnitude: ");
  Serial.print(magnitude, 2);

  Serial.println(" m/s²");

  delay(500);
}