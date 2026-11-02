// [ru] AlashServo: разные формы движения. Серво едет туда и обратно с разной кривой: сначала равномерно, потом плавный старт, торможение и отскок.
//      В Serial Monitor (115200) видно, какая кривая сейчас.
// [kk] AlashServo: қозғалыстың әртүрлі пішіндері. Серво әртүрлі қисықпен алға-артқа жүреді: алдымен бірқалыпты, сосын баяу бастау, тежеу және секіру.
//      Serial Monitor-да (115200) қазір қай қисық екені көрінеді.
// [en] AlashServo: different shapes of motion. The servo goes back and forth with different curves: first uniform, then a smooth start, braking and a bounce.
//      The Serial Monitor (115200) shows which curve is current.
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
