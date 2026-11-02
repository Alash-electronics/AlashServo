// [ru] AlashServo: качание, как у радара или головы робота. sweep() сам разворачивает серво на краях, пока не вызовут stop().
//      Параллельно программа мигает светодиодом без delay(): серво ей не мешает.
// [kk] AlashServo: радар немесе робот басы сияқты тербеліс. sweep() stop() шақырылғанша серваны шеттерінде өзі бұрады.
//      Қатар бағдарлама delay() жоқ жарықдиодты жыпылықтатады: серво оған кедергі жасамайды.
// [en] AlashServo: a sweep, like a radar or a robot head. sweep() turns the servo around at the ends by itself until stop() is called.
//      In parallel the program blinks an LED without delay(): the servo does not get in the way.
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
