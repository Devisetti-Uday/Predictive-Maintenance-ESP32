#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#define MACHINE_DHT_PIN 4
#define AMBIENT_DHT_PIN 5
#define DHT_TYPE DHT22
#define GREEN_PIN 25
#define BLUE_PIN 26
#define RED_PIN 27
#define BUZZER_PIN 33
Adafruit_MPU6050 mpu;
DHT machineDHT(MACHINE_DHT_PIN, DHT_TYPE);
DHT ambientDHT(AMBIENT_DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
const unsigned long MONITOR_INTERVAL = 10000UL;
const int CALIBRATION_SAMPLES = 10;
float baselineAx = 0;
float baselineAy = 0;
float baselineAz = 0;
float baselineTempDifference = 0;
float currentMachineTemp = 0;
float currentAmbientTemp = 0;
float currentTempDifference = 0;
float currentVibration = 0;
const int HISTORY_SIZE = 5;
float vibrationHistory[HISTORY_SIZE];
float tempDifferenceHistory[HISTORY_SIZE];
int historyCount = 0;
int historyIndex = 0;
const int STATUS_HISTORY_SIZE = 3;
enum MachineStatus {
  HEALTHY,
  WARNING,
  CRITICAL
};
MachineStatus statusHistory[STATUS_HISTORY_SIZE];
int statusHistoryCount = 0;
int statusHistoryIndex = 0;
MachineStatus machineStatus = HEALTHY;
float healthScore = 100;
unsigned long lastMonitoringTime = 0;
void setRGB(bool red, bool green, bool blue) {
  digitalWrite(RED_PIN, red ? LOW : HIGH);
  digitalWrite(GREEN_PIN, green ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blue ? LOW : HIGH);
}
void updateIndicators() {
  if (machineStatus == HEALTHY) {
    setRGB(false, true, false);
    noTone(BUZZER_PIN);
  }
  else if (machineStatus == WARNING) {
    setRGB(true, true, false);
    noTone(BUZZER_PIN);
  }
  else {
    setRGB(true, false, false);
    tone(BUZZER_PIN, 1000);
  }
}
float readVibration() {
  sensors_event_t a, g, tempEvent;
  mpu.getEvent(&a, &g, &tempEvent);
  float dx = a.acceleration.x - baselineAx;
  float dy = a.acceleration.y - baselineAy;
  float dz = a.acceleration.z - baselineAz;
  float vibration = sqrt(dx * dx + dy * dy + dz * dz);
  return vibration;
}
void calibrateMachine() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Normal Machine");
  Serial.println();
  Serial.println("================================");
  Serial.println(" MACHINE CALIBRATION");
  Serial.println("================================");
  Serial.println("Keep machine in normal condition.");
  Serial.println();
  float sumAx = 0;
  float sumAy = 0;
  float sumAz = 0;
  float sumTempDifference = 0;
  int validSamples = 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    sensors_event_t a, g, tempEvent;
    mpu.getEvent(&a, &g, &tempEvent);
    float machineTemp = machineDHT.readTemperature();
    float ambientTemp = ambientDHT.readTemperature();
    if (!isnan(machineTemp) && !isnan(ambientTemp)) {
      float tempDifference = machineTemp - ambientTemp;
      sumAx += a.acceleration.x;
      sumAy += a.acceleration.y;
      sumAz += a.acceleration.z;
      sumTempDifference += tempDifference;
      validSamples++;
      Serial.print("Calibration Sample ");
      Serial.print(i + 1);
      Serial.print(" | Machine T = ");
      Serial.print(machineTemp, 2);
      Serial.print(" C | Ambient T = ");
      Serial.print(ambientTemp, 2);
      Serial.print(" C | Difference = ");
      Serial.print(tempDifference, 2);
      Serial.print(" C | Ax = ");
      Serial.print(a.acceleration.x, 2);
      Serial.print(" | Ay = ");
      Serial.print(a.acceleration.y, 2);
      Serial.print(" | Az = ");
      Serial.println(a.acceleration.z, 2);
    }
    delay(1000);
  }
  if (validSamples > 0) {
    baselineAx = sumAx / validSamples;
    baselineAy = sumAy / validSamples;
    baselineAz = sumAz / validSamples;
    baselineTempDifference = sumTempDifference / validSamples;
  }
  Serial.println();
  Serial.println("BASELINE ESTABLISHED");
  Serial.print("Baseline Temperature Difference : ");
  Serial.print(baselineTempDifference, 2);
  Serial.println(" C");
  Serial.print("Baseline Ax : ");
  Serial.println(baselineAx, 2);
  Serial.print("Baseline Ay : ");
  Serial.println(baselineAy, 2);
  Serial.print("Baseline Az : ");
  Serial.println(baselineAz, 2);
  Serial.println("================================");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Baseline Set");
  lcd.setCursor(0, 1);
  lcd.print("Machine Healthy");
  setRGB(false, true, false);
  delay(2000);
}
void confirmStatus(MachineStatus detectedStatus) {
  statusHistory[statusHistoryIndex] = detectedStatus;
  statusHistoryIndex++;
  if (statusHistoryIndex >= STATUS_HISTORY_SIZE)
    statusHistoryIndex = 0;
  if (statusHistoryCount < STATUS_HISTORY_SIZE)
    statusHistoryCount++;
  int warningCount = 0;
  int criticalCount = 0;
  for (int i = 0; i < statusHistoryCount; i++) {
    if (statusHistory[i] == WARNING)
      warningCount++;
    if (statusHistory[i] == CRITICAL)
      criticalCount++;
  }
  if (criticalCount >= 3) {
    machineStatus = CRITICAL;
  }
  else if (criticalCount >= 2 || warningCount >= 2) {
    machineStatus = WARNING;
  }
  else {
    machineStatus = HEALTHY;
  }
}
void analyzeMachine() {
  healthScore = 100;
  currentTempDifference =
    currentMachineTemp - currentAmbientTemp;
  float thermalDeviation =
    currentTempDifference - baselineTempDifference;
  if (thermalDeviation < 0)
    thermalDeviation = 0;
  if (thermalDeviation >= 2)
    healthScore -= 10;
  if (thermalDeviation >= 5)
    healthScore -= 20;
  if (thermalDeviation >= 8)
    healthScore -= 30;
  if (currentVibration >= 0.2)
    healthScore -= 10;
  if (currentVibration >= 0.5)
    healthScore -= 20;
  if (currentVibration >= 1.0)
    healthScore -= 30;
  if (historyCount >= 3) {
    int oldIndex =
      (historyIndex + HISTORY_SIZE - 3) % HISTORY_SIZE;
    int recentIndex =
      (historyIndex + HISTORY_SIZE - 1) % HISTORY_SIZE;
    float oldVibration =
      vibrationHistory[oldIndex];
    float recentVibration =
      vibrationHistory[recentIndex];
    float vibrationIncrease =
      recentVibration - oldVibration;
    if (vibrationIncrease > 0.2)
      healthScore -= 10;
    if (vibrationIncrease > 0.5)
      healthScore -= 15;
  }
  if (healthScore < 0)
    healthScore = 0;
  if (healthScore > 100)
    healthScore = 100;
  MachineStatus detectedStatus;
  if (healthScore >= 80)
    detectedStatus = HEALTHY;
  else if (healthScore >= 50)
    detectedStatus = WARNING;
  else
    detectedStatus = CRITICAL;
  confirmStatus(detectedStatus);
  updateIndicators();
}
void displayMachineStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("M:");
  lcd.print(currentMachineTemp, 1);
  lcd.print(" A:");
  lcd.print(currentAmbientTemp, 1);
  lcd.setCursor(0, 1);
  if (machineStatus == HEALTHY)
    lcd.print("HEALTHY ");
  else if (machineStatus == WARNING)
    lcd.print("MAINTAIN ");
  else
    lcd.print("CRITICAL ");
  lcd.print((int)healthScore);
}
void monitorMachine() {
  sensors_event_t a, g, tempEvent;
  mpu.getEvent(&a, &g, &tempEvent);
  currentMachineTemp =
    machineDHT.readTemperature();
  currentAmbientTemp =
    ambientDHT.readTemperature();
  if (isnan(currentMachineTemp) ||
      isnan(currentAmbientTemp)) {
    Serial.println("Temperature sensor reading failed!");
    return;
  }
  currentTempDifference =
    currentMachineTemp - currentAmbientTemp;
  currentVibration =
    readVibration();
  vibrationHistory[historyIndex] =
    currentVibration;
  tempDifferenceHistory[historyIndex] =
    currentTempDifference;
  historyIndex++;
  if (historyIndex >= HISTORY_SIZE)
    historyIndex = 0;
  if (historyCount < HISTORY_SIZE)
    historyCount++;
  analyzeMachine();
  Serial.println();
  Serial.println("========== MACHINE CHECK ==========");
  Serial.print("Machine Temperature : ");
  Serial.print(currentMachineTemp, 2);
  Serial.println(" C");
  Serial.print("Ambient Temperature : ");
  Serial.print(currentAmbientTemp, 2);
  Serial.println(" C");
  Serial.print("Temperature Difference : ");
  Serial.print(currentTempDifference, 2);
  Serial.println(" C");
  Serial.print("Baseline Difference : ");
  Serial.print(baselineTempDifference, 2);
  Serial.println(" C");
  float displayedThermalDeviation =
    currentTempDifference - baselineTempDifference;
  if (displayedThermalDeviation < 0)
    displayedThermalDeviation = 0;
  Serial.print("Thermal Deviation : ");
  Serial.print(displayedThermalDeviation, 2);
  Serial.println(" C");
  Serial.print("Vibration : ");
  Serial.println(currentVibration, 2);
  Serial.print("Health Score : ");
  Serial.println(healthScore, 2);
  Serial.print("Current Detection : ");
  if (healthScore >= 80)
    Serial.println("HEALTHY");
  else if (healthScore >= 50)
    Serial.println("MAINTENANCE REQUIRED");
  else
    Serial.println("CRITICAL");
  Serial.print("Confirmed Status : ");
  if (machineStatus == HEALTHY)
    Serial.println("HEALTHY");
  else if (machineStatus == WARNING)
    Serial.println("MAINTENANCE REQUIRED");
  else
    Serial.println("CRITICAL - FAILURE RISK");
  Serial.print("Observations Used For Confirmation : ");
  Serial.println(statusHistoryCount);
  Serial.println("===================================");
  displayMachineStatus();
}
void setup() {
  Serial.begin(115200);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  setRGB(false, false, false);
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  machineDHT.begin();
  ambientDHT.begin();
  if (!mpu.begin()) {
    lcd.clear();
    lcd.print("MPU6050 Error");
    Serial.println("MPU6050 not found!");
    while (1);
  }
  mpu.setAccelerometerRange(
    MPU6050_RANGE_8_G
  );
  mpu.setGyroRange(
    MPU6050_RANGE_500_DEG
  );
  mpu.setFilterBandwidth(
    MPU6050_BAND_21_HZ
  );
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Predictive");
  lcd.setCursor(0, 1);
  lcd.print("Maintenance");
  delay(2000);
  calibrateMachine();
  monitorMachine();
  lastMonitoringTime = millis();
}
void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastMonitoringTime
      >= MONITOR_INTERVAL) {
    lastMonitoringTime = currentTime;
    monitorMachine();
  }
}