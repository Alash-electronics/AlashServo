// Тесты AlashServo без железа, вывод через встроенную Servo.h (AVR, ESP8266 и др.): имитация библиотеки Servo.
//   g++ -std=c++17 -Wall -I mock -I ../../src test_servo.cpp ../../src/AlashServo.cpp && ./a.out
#include "servo_cases.h"
int main() { return runCases(); }
