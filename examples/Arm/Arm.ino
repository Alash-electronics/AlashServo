// AlashServo: рука-манипулятор из трёх серво. AlashServoGroup двигает их вместе, все приезжают в цель одновременно.
// Подключение: основание на пин 9, плечо на 10, схват на 11 (на ESP32 пины 13, 14, 27). Питание серво от отдельного источника.
#include <AlashServo.h>

#if defined(ARDUINO_ARCH_ESP32)
const uint8_t PINS[3] = {13, 14, 27};
#else
const uint8_t PINS[3] = {9, 10, 11};
#endif

AlashServo joints[3];
AlashServoGroup arm(joints, 3);

void setup() {
  for (uint8_t i = 0; i < 3; i++) {
    joints[i].attach(PINS[i]);
    joints[i].setSpeed(60);              // не быстрее 60 градусов в секунду: рука двигается мягко
    joints[i].write(90);
  }
  joints[2].setLimits(30, 120);          // схват не должен сжиматься сильнее: защита механики
}

void loop() {
  arm.update();
  static uint8_t step = 0;
  static uint32_t pauseUntil = 0;
  if (!arm.moving() && millis() > pauseUntil) {
    static const float poses[3][3] = {{45, 120, 60}, {135, 60, 110}, {90, 90, 90}};   // основание, плечо, схват
    arm.moveTo(poses[step]);             // время берётся по самому долгому движению при скорости 60 град/с
    step = (step + 1) % 3;
    pauseUntil = millis() + 1500;
  }
}
