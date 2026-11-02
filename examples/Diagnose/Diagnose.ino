// [ru] AlashServo: самопроверка. Печатает настройки и прокатывает серво в середину и на 45 градусов в обе стороны.
//      Если серво не двигается или дёргается, прочтите подсказки, которые появятся в Serial Monitor (115200).
// [kk] AlashServo: өзін-өзі тексеру. Баптауларды басып шығарады және серваны ортаға, сосын екі жаққа 45 градусқа жүргізеді.
//      Серво қозғалмаса немесе тартылса, Serial Monitor-да (115200) шығатын кеңестерді оқыңыз.
// [en] AlashServo: self-check. Prints the settings and runs the servo to the middle and 45 degrees each way.
//      If the servo does not move or jitters, read the tips that appear in the Serial Monitor (115200).
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
