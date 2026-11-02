// AlashServo: качание, как у радара или головы робота. sweep() сам разворачивает серво на краях, пока не вызовут stop().
// Параллельно программа мигает светодиодом без delay(): серво ей не мешает.
#include <AlashServo.h>

#if defined(ARDUINO_ARCH_ESP32)
const uint8_t SERVO_PIN = 13, LED = 2;
#else
const uint8_t SERVO_PIN = 9, LED = 13;
#endif

AlashServo radar;

void setup() {
  pinMode(LED, OUTPUT);
  radar.attach(SERVO_PIN);
  radar.setLimits(20, 160);              // защита механики: за эти углы серво не поедет
  radar.sweep(20, 160, 3000);            // полный круг туда-обратно за 3 секунды
}

void loop() {
  radar.update();
  digitalWrite(LED, (millis() / 250) % 2);   // мигаем, пока серво качается
}
