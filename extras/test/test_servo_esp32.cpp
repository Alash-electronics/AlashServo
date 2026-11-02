// Тесты AlashServo без железа, ESP32 с ядром 3.x: имитация ledcAttach / ledcWrite / ledcDetach.
//   g++ -std=c++17 -Wall -DARDUINO_ARCH_ESP32 -DESP_ARDUINO_VERSION_MAJOR=3 -I mock -I ../../src test_servo_esp32.cpp ../../src/AlashServo.cpp && ./a.out
#include "servo_cases.h"
int main() { return runCases(); }
