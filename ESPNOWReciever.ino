#include <WiFi.h>
#include <esp_now.h>

// ==== Motor pins ====
// Left SmartElex
#define L_DIR 12
#define L_PWM 14

// Right SmartElex
#define R_DIR 27
#define R_PWM 26

// ==== Receiver PWM input pins ====
#define RX_CH1 34   // Steering
#define RX_CH2 35   // Throttle

// ==== LEDC settings ====
#define PWM_FREQ 25000
#define PWM_RES 8

// LEDC channels
#define CH_L 0
#define CH_R 1

// ==== MOTOR REVERSE FLAGS ====
bool REVERSE_LEFT  = false;
bool REVERSE_RIGHT = true;

// ==== ESP-NOW data ====
typedef struct {
  int ch1;
  int ch2;
  int ch3;
  int ch4;
} ChannelData;

ChannelData data;

// ==== FAILSAFE ====
unsigned long lastPacketTime = 0;
const unsigned long failsafeTimeout = 150;

// ==== Motor control (DIR + PWM) ====
void driveMotor(int dirPin, int pwmChannel, int value, bool reverse) {
  int speed = constrain(value, -255, 255);

  if (abs(speed) < 10) {
    ledcWrite(pwmChannel, 0);
    return;
  }

  if (reverse) speed = -speed;

  if (speed > 0) {
    digitalWrite(dirPin, HIGH);
    ledcWrite(pwmChannel, speed);
  } else {
    digitalWrite(dirPin, LOW);
    ledcWrite(pwmChannel, -speed);
  }
}

// ==== Apply tank steering ====
void applyTankMix(int throttle, int steering) {
  steering /= 2;

  int leftPower  = constrain(throttle + steering, -255, 255);
  int rightPower = constrain(throttle - steering, -255, 255);

  driveMotor(L_DIR, CH_L, leftPower,  REVERSE_LEFT);
  driveMotor(R_DIR, CH_R, rightPower, REVERSE_RIGHT);
}

// ==== ESP-NOW callback (LOGIC UNCHANGED) ====
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
  lastPacketTime = millis();
}

// ==== Read receiver PWM ====
bool readReceiver(int &throttleOut, int &steeringOut) {
  int ch1 = pulseIn(RX_CH1, HIGH, 25000); // 25 ms timeout
  int ch2 = pulseIn(RX_CH2, HIGH, 25000);

  if (ch1 < 900 || ch1 > 2100 || ch2 < 900 || ch2 > 2100) {
    return false; // no valid receiver signal
  }

  steeringOut = map(ch1, 1000, 2000, -255, 255);
  throttleOut = map(ch2, 1000, 2000, -255, 255);
  return true;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(L_DIR, OUTPUT);
  pinMode(R_DIR, OUTPUT);

  pinMode(RX_CH1, INPUT);
  pinMode(RX_CH2, INPUT);

  ledcSetup(CH_L, PWM_FREQ, PWM_RES);
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);

  ledcAttachPin(L_PWM, CH_L);
  ledcAttachPin(R_PWM, CH_R);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Receiver + ESP-NOW Hybrid Ready");
}

void loop() {
  int throttle, steering;

  // ===== Receiver has PRIORITY =====
  if (readReceiver(throttle, steering)) {
    applyTankMix(throttle, steering);
    return;
  }

  // ===== Fall back to ESP-NOW =====
  if (millis() - lastPacketTime <= failsafeTimeout) {
    int espThrottle = map(data.ch3, 1000, 2000, -255, 255);
    int espSteering = map(data.ch1, 1000, 2000, -255, 255);
    applyTankMix(espThrottle, espSteering);
  } else {
    ledcWrite(CH_L, 0);
    ledcWrite(CH_R, 0);
  }
}
