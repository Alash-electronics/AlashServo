// AlashServo: самопроверка. Печатает настройки и прокатывает серво в середину и на 45 градусов в обе стороны.
// Если серво не двигается или дёргается, прочтите подсказки, которые появятся в Serial Monitor (115200).
#include <AlashServo.h>

#if defined(ARDUINO_ARCH_ESP32)
const uint8_t SERVO_PIN = 13;
#else
const uint8_t SERVO_PIN = 9;
#endif

AlashServo servo;

void setup() {
  Serial.begin(115200);
  servo.setLanguage(AlashServo::LANG_RU);      // LANG_KK или LANG_EN
  servo.attach(SERVO_PIN);
  servo.diagnose(Serial);
  if (servo.error() != AlashServo::ERR_OK) Serial.println(servo.errorText());
}

void loop() {}
