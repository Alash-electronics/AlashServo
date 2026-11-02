// Имитация встроенной библиотеки Servo (AVR, ESP8266): то же поведение, что видит AlashServo.
#pragma once
#include "Arduino.h"
class Servo {
 public:
  uint8_t attach(int pin, int minUs = 544, int maxUs = 2400) {
    if (g_attachFail) return 255;
    if (_on) detach();
    _pin = pin; _on = true; evAttach(pin); (void)minUs; (void)maxUs; return 0;
  }
  void detach() { if (_on) { evDetach(_pin); _on = false; } }
  bool attached() const { return _on; }
  void writeMicroseconds(int us) { if (_on) evWrite(_pin, us); }
 private:
  int _pin = -1; bool _on = false;
};
