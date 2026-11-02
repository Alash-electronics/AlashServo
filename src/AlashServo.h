// AlashServo: плавный сервопривод без delay(): moveTo(угол, время, кривая), скорость, пределы, реверс, авто-отключение импульсов.
// Одинаково на Arduino AVR, ESP8266 (встроенная Servo.h) и ESP32 (аппаратный ШИМ LEDC, отдельная библиотека не нужна).
// Кривые движения по Р. Пеннеру. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashServo
#ifndef ALASH_SERVO_H
#define ALASH_SERVO_H

#include <Arduino.h>
#if !defined(ARDUINO_ARCH_ESP32)
#include <Servo.h>
#endif

class AlashServo {
 public:
  // Форма движения: LINEAR равномерно, *_IN медленный старт, *_OUT медленное торможение, SMOOTH плавный старт и финиш, BOUNCE_OUT отскок в конце
  enum Easing : uint8_t { LINEAR = 0, QUAD_IN, QUAD_OUT, QUAD_IN_OUT, SMOOTH, BOUNCE_OUT };
  enum Error : uint8_t { ERR_OK = 0, ERR_NOT_ATTACHED, ERR_BAD_PIN, ERR_BAD_PULSE, ERR_RANGE, ERR_NO_CHANNEL, ERR_BAD_VALUE };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  AlashServo() {}
  ~AlashServo() { detach(); }

  // Подключить к пину. minUs/maxUs: длина импульса для 0 и крайнего угла (для SG90 обычно 500..2400), travelDeg: полный ход (180, у некоторых 270)
  bool attach(uint8_t pin, uint16_t minUs = 544, uint16_t maxUs = 2400, float travelDeg = 180.0f);
  void detach();
  bool attached() const { return _attached; }

  // ---- Движение -------------------------------------------------------------------------
  void write(float angle);                                          // сразу в угол, без плавности
  void writeMicroseconds(uint16_t us);                              // напрямую длина импульса
  void moveTo(float angle, uint32_t durationMs = 0, Easing e = LINEAR);   // плавно; durationMs = 0: по скорости setSpeed() (если скорость 0, сразу)
  void moveBy(float delta, uint32_t durationMs = 0, Easing e = LINEAR) { moveTo(_target + delta, durationMs, e); }
  void center() { moveTo((_lo + _hi) * 0.5f); }                    // к середине хода
  void stop();                                                      // остановиться где стоит
  void sweep(float from, float to, uint32_t periodMs, Easing e = SMOOTH);   // качаться туда-сюда, пока не вызовут stop()
  bool update();                                                    // вызывайте в loop(); возвращает true, пока серво едет
  bool moving() const { return _moving; }
  void onFinish(void (*cb)()) { _cb = cb; }                         // вызвать, когда доехали

  // ---- Настройка -------------------------------------------------------------------------
  void setSpeed(float degreesPerSecond);                            // скорость для moveTo без времени; 0 = без ограничения
  bool setLimits(float minAngle, float maxAngle);                   // защита механики: за эти углы серво не поедет
  void setReverse(bool reverse) { _reverse = reverse; }             // направление наоборот
  void setTrim(float degrees) { _trim = degrees; }                  // поправка нуля (если рычаг стоит чуть криво)
  void setIdleDetach(uint32_t ms) { _idleMs = ms; }                 // через ms после остановки отпустить сигнал (серво не гудит, но и не держит нагрузку); 0 = не отпускать

  // ---- Состояние ---------------------------------------------------------------------------
  float angle() const { return _cur; }                              // текущий угол
  float target() const { return _target; }
  float progress() const;                                           // 0..1 для текущего движения
  uint16_t microseconds() const { return _us; }                     // последняя длина импульса
  uint32_t travelTimeMs(float toAngle) const;                       // сколько ехать до угла при заданной скорости
  bool active() const { return _active; }                           // сигнал сейчас подаётся

  // ---- Диагностика ------------------------------------------------------------------
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);                                        // печатает настройки и ставит серво в 90, 45, 135 (блокирующий тест)

 private:
  friend class AlashServoGroup;
  float clampAngle(float a);
  uint16_t usFor(float a) const;
  void apply();
  bool backendAttach();
  void backendWrite(uint16_t us);
  void backendDetach();
  void startMove(float angle, uint32_t durationMs, Easing e);

  bool _attached = false, _active = false, _moving = false, _reverse = false, _sweeping = false, _haveUs = false;
  uint8_t _pin = 255;
  uint16_t _minUs = 544, _maxUs = 2400, _us = 0;
  float _travel = 180.0f, _lo = 0.0f, _hi = 180.0f, _trim = 0.0f, _speed = 0.0f;
  float _cur = 90.0f, _target = 90.0f, _start = 90.0f, _swA = 0, _swB = 0;
  uint32_t _t0 = 0, _dur = 0, _idleMs = 0, _idleAt = 0, _swPeriod = 0;
  Easing _ease = LINEAR;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  void (*_cb)() = nullptr;
#if !defined(ARDUINO_ARCH_ESP32)
  Servo _servo;
#elif !defined(ESP_ARDUINO_VERSION_MAJOR) || ESP_ARDUINO_VERSION_MAJOR < 3
  int8_t _channel = -1;           // ядро ESP32 2.x: номер канала LEDC
#endif
};

// Несколько серво вместе (рука-манипулятор, шагающий робот): все приезжают в свои углы одновременно.
class AlashServoGroup {
 public:
  AlashServoGroup(AlashServo *servos, uint8_t count) : _s(servos), _n(count) {}
  void moveTo(const float *angles, uint32_t durationMs = 0, AlashServo::Easing e = AlashServo::SMOOTH);   // durationMs = 0: время по самому долгому из движений
  bool update();                       // true, пока хоть одно едет
  bool moving() const;
  void stop();
  void center();
 private:
  AlashServo *_s;
  uint8_t _n;
};

#endif
