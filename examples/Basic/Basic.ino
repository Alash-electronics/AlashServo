// AlashServo: самый простой пример. Серво плавно едет в три положения, программа при этом не стоит.
//
// Подключение: коричневый или чёрный провод серво к GND, красный к 5 В (лучше от отдельного источника, не от пина Arduino),
// оранжевый или жёлтый (сигнал) к пину 9. На ESP32 возьмите, например, пин 13.
#include <AlashServo.h>

#if defined(ARDUINO_ARCH_ESP32)
const uint8_t SERVO_PIN = 13;
#else
const uint8_t SERVO_PIN = 9;
#endif

AlashServo servo;

void setup() {
  servo.attach(SERVO_PIN);               // для SG90 можно attach(пин, 500, 2400)
  servo.write(0);                        // первое положение сразу
}

void loop() {
  static const float angles[] = {0, 90, 180, 90};
  static uint8_t i = 0;
  static uint32_t pauseUntil = 0;

  servo.update();                        // вызывайте в каждом проходе loop()
  if (!servo.moving() && millis() > pauseUntil) {
    i = (i + 1) % 4;
    servo.moveTo(angles[i], 1500);       // доехать за 1,5 секунды
    pauseUntil = millis() + 2500;        // 1,5 с езды и секунда паузы
  }
}
