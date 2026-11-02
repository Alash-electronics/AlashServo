// Имитация Arduino.h и выхода серво для тестов AlashServo. Все варианты (Servo.h, LEDC ядра 3 и ядра 2) пишут один журнал g_ev.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#define PROGMEM
inline uint32_t g_ms = 1000;
inline uint32_t millis() { return g_ms; }
inline void delay(uint32_t ms) { g_ms += ms; }

struct Ev { int kind; int pin; int us; };           // kind: 0 attach, 1 write, 2 detach
inline std::vector<Ev> g_ev;
inline bool g_attachFail = false;                     // имитация нехватки каналов
inline int g_liveAttached = 0;                       // сколько пинов сейчас подключено
inline void evAttach(int pin) { g_ev.push_back({0, pin, 0}); g_liveAttached++; }
inline void evWrite(int pin, int us) { g_ev.push_back({1, pin, us}); }
inline void evDetach(int pin) { g_ev.push_back({2, pin, 0}); g_liveAttached--; }

class Print {
 public:
  std::string text;
  void put(const char *s) { printf("%s", s); text += s; }
  void print(const char *s) { put(s); }
  void print(const __FlashStringHelper *s) { put((const char *)s); }
  void println(const __FlashStringHelper *s) { put((const char *)s); put("\n"); }
  void print(long v) { char b[24]; snprintf(b, sizeof b, "%ld", v); put(b); }
  void println(long v) { print(v); put("\n"); }
  void print(double v, int d = 2) { char b[40]; snprintf(b, sizeof b, "%.*f", d, v); put(b); }
  void println(double v, int d = 2) { print(v, d); put("\n"); }
  void println(const char *s = "") { put(s); put("\n"); }
};

#if defined(ARDUINO_ARCH_ESP32)
// ---- ESP32: LEDC ----
inline uint32_t dutyToUs(uint32_t duty) { return (uint32_t)((uint64_t)duty * 20000 / 16384); }
#if ESP_ARDUINO_VERSION_MAJOR >= 3
inline bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t res) { if (g_attachFail || freq != 50 || res != 14) return false; evAttach(pin); return true; }
inline bool ledcWrite(uint8_t pin, uint32_t duty) { evWrite(pin, dutyToUs(duty)); return true; }
inline bool ledcDetach(uint8_t pin) { evDetach(pin); return true; }
#else
inline int g_chPin[16];
inline int g_chCfg[16];
inline double ledcSetup(uint8_t ch, double freq, uint8_t res) { if (g_attachFail) return 0; g_chCfg[ch] = (freq == 50 && res == 14); return freq; }
inline void ledcAttachPin(uint8_t pin, uint8_t ch) { g_chPin[ch] = pin; evAttach(pin); }
inline void ledcWrite(uint8_t ch, uint32_t duty) { if (g_chCfg[ch]) evWrite(g_chPin[ch], dutyToUs(duty)); }
inline void ledcDetachPin(uint8_t pin) { evDetach(pin); }
#endif
#endif
