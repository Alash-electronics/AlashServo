// Тесты AlashServo без железа, ESP32 с ядром 2.x: каналы LEDC выбирает библиотека (ledcSetup / ledcAttachPin).
//   g++ -std=c++17 -Wall -DARDUINO_ARCH_ESP32 -DESP_ARDUINO_VERSION_MAJOR=2 -I mock -I ../../src test_servo_esp32_core2.cpp ../../src/AlashServo.cpp && ./a.out
#include "servo_cases.h"
int main() {
  int r = runCases();
  // 16 каналов: семнадцатое серво получает ERR_NO_CHANNEL, освободив одно, подключается
  g_ev.clear(); g_attachFail = false;
  static AlashServo many[17];
  for (int i = 0; i < 17; i++) { many[i].attach(i); many[i].write(90); }
  int active = 0; for (int i = 0; i < 17; i++) if (many[i].active()) active++;
  CHECK(active == 16 && many[16].error() == AlashServo::ERR_NO_CHANNEL);
  many[0].detach(); many[16].clearError(); many[16].write(45);
  CHECK(many[16].active());
  for (int i = 0; i < 17; i++) many[i].detach();
  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  return r;
}
