// AlashServo: разные формы движения. Серво едет туда и обратно с разной кривой: сначала равномерно, потом плавный старт, торможение и отскок.
// В Serial Monitor (115200) видно, какая кривая сейчас.
#include <AlashServo.h>

#if defined(ARDUINO_ARCH_ESP32)
const uint8_t SERVO_PIN = 13;
#else
const uint8_t SERVO_PIN = 9;
#endif

AlashServo servo;
const AlashServo::Easing kinds[] = {AlashServo::LINEAR, AlashServo::QUAD_IN, AlashServo::QUAD_OUT, AlashServo::SMOOTH, AlashServo::BOUNCE_OUT};
const char *const names[] = {"LINEAR", "QUAD_IN", "QUAD_OUT", "SMOOTH", "BOUNCE_OUT"};

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN);
  servo.write(20);
}

void loop() {
  static uint8_t k = 0;
  static bool toEnd = true;
  servo.update();
  if (!servo.moving()) {
    delay(300);                          // короткая пауза в конце, просто чтобы было видно
    Serial.println(names[k]);
    servo.moveTo(toEnd ? 160 : 20, 1200, kinds[k]);
    if (!toEnd) k = (k + 1) % 5;         // после возврата переходим к следующей кривой
    toEnd = !toEnd;
  }
}
