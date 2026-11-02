// MIT License. Alash Electronics.
#include "AlashServo.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static inline bool finiteF(float v) { return v == v && (v - v) == 0.0f; }

// ---- Выход сигнала: Servo.h (AVR, ESP8266 и др.) или LEDC (ESP32) ----------------------------------
#if defined(ARDUINO_ARCH_ESP32)
#define ALASH_SERVO_HZ 50
#define ALASH_SERVO_BITS 14
static inline uint32_t dutyFromUs(uint16_t us) {      // с округлением: шаг LEDC около 1,2 мкс
  const uint32_t period = 1000000UL / ALASH_SERVO_HZ;
  return ((uint32_t)us * (1UL << ALASH_SERVO_BITS) + period / 2) / period;
}
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
bool AlashServo::backendAttach() {
  if (!ledcAttach(_pin, ALASH_SERVO_HZ, ALASH_SERVO_BITS)) { _err = ERR_NO_CHANNEL; return false; }
  return true;
}
void AlashServo::backendWrite(uint16_t us) { ledcWrite(_pin, dutyFromUs(us)); }
void AlashServo::backendDetach() { ledcDetach(_pin); }
#else
static uint16_t s_usedChannels = 0;             // ядро ESP32 2.x: каналы выбираем сами, с 15 вниз, чтобы не мешать обычному ШИМ
bool AlashServo::backendAttach() {
  int ch = -1;
  for (int c = 15; c >= 0; c--) if (!(s_usedChannels & (1u << c))) { ch = c; break; }
  if (ch < 0) { _err = ERR_NO_CHANNEL; return false; }
  if (ledcSetup(ch, ALASH_SERVO_HZ, ALASH_SERVO_BITS) == 0) { _err = ERR_NO_CHANNEL; return false; }
  ledcAttachPin(_pin, ch);
  s_usedChannels |= (1u << ch);
  _channel = (int8_t)ch;
  return true;
}
void AlashServo::backendWrite(uint16_t us) { if (_channel >= 0) ledcWrite(_channel, dutyFromUs(us)); }
void AlashServo::backendDetach() {
  if (_channel < 0) return;
  ledcDetachPin(_pin);
  s_usedChannels &= ~(1u << _channel);
  _channel = -1;
}
#endif
#else
bool AlashServo::backendAttach() {
  _servo.attach(_pin, _minUs, _maxUs);
  if (!_servo.attached()) { _err = ERR_NO_CHANNEL; return false; }
  return true;
}
void AlashServo::backendWrite(uint16_t us) { _servo.writeMicroseconds(us); }
void AlashServo::backendDetach() { _servo.detach(); }
#endif

// ---- Кривые движения (по Р. Пеннеру), t от 0 до 1 ----------------------------------------------------
static float easeCurve(AlashServo::Easing e, float t) {
  switch (e) {
    case AlashServo::QUAD_IN: return t * t;
    case AlashServo::QUAD_OUT: return t * (2.0f - t);
    case AlashServo::QUAD_IN_OUT: return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    case AlashServo::SMOOTH: return t * t * (3.0f - 2.0f * t);
    case AlashServo::BOUNCE_OUT:
      if (t < 1.0f / 2.75f) return 7.5625f * t * t;
      if (t < 2.0f / 2.75f) { t -= 1.5f / 2.75f; return 7.5625f * t * t + 0.75f; }
      if (t < 2.5f / 2.75f) { t -= 2.25f / 2.75f; return 7.5625f * t * t + 0.9375f; }
      t -= 2.625f / 2.75f; return 7.5625f * t * t + 0.984375f;
    case AlashServo::LINEAR: default: return t;
  }
}

// ---- Подключение --------------------------------------------------------------------------------
bool AlashServo::attach(uint8_t pin, uint16_t minUs, uint16_t maxUs, float travelDeg) {
  if (pin == 255) { _err = ERR_BAD_PIN; return false; }
  if (minUs < 400 || maxUs > 2700 || maxUs < minUs + 100) { _err = ERR_BAD_PULSE; return false; }
  if (!finiteF(travelDeg) || travelDeg < 10.0f || travelDeg > 720.0f) { _err = ERR_BAD_VALUE; return false; }
  detach();
  _pin = pin; _minUs = minUs; _maxUs = maxUs; _travel = travelDeg;
  _lo = 0; _hi = travelDeg; _cur = _target = _start = travelDeg * 0.5f;
  _haveUs = false; _moving = false; _sweeping = false; _active = false;
  _attached = true;
  _err = ERR_OK;
  return true;           // импульсы пойдут при первой команде: серво не дёргается от предполагаемого положения
}

void AlashServo::detach() {
  if (_active) backendDetach();
  _active = false; _attached = false; _moving = false; _sweeping = false; _haveUs = false;
}

// ---- Движение -----------------------------------------------------------------------------------
float AlashServo::clampAngle(float a) {
  if (!finiteF(a)) { _err = ERR_BAD_VALUE; return _target; }
  if (a < _lo) { _err = ERR_RANGE; return _lo; }
  if (a > _hi) { _err = ERR_RANGE; return _hi; }
  return a;
}

uint16_t AlashServo::usFor(float a) const {
  float phys = (_reverse ? _travel - a : a) + _trim;
  if (phys < 0) phys = 0;
  if (phys > _travel) phys = _travel;
  return (uint16_t)(_minUs + (phys / _travel) * (float)(_maxUs - _minUs) + 0.5f);
}

void AlashServo::apply() {
  uint16_t us = usFor(_cur);
  bool fresh = false;
  if (!_active) {
    if (!backendAttach()) return;
    _active = true; fresh = true;
  }
  if (fresh || !_haveUs || us != _us) { backendWrite(us); _us = us; _haveUs = true; }
}

void AlashServo::write(float angle) {
  if (!_attached) { _err = ERR_NOT_ATTACHED; return; }
  float a = clampAngle(angle);
  _sweeping = false; _moving = false;
  _cur = _target = _start = a;
  apply();
  _idleAt = millis();
}

void AlashServo::writeMicroseconds(uint16_t us) {
  if (!_attached) { _err = ERR_NOT_ATTACHED; return; }
  if (us < 400 || us > 2700) { _err = ERR_BAD_PULSE; return; }
  _sweeping = false; _moving = false;
  if (!_active) { if (!backendAttach()) return; _active = true; }
  backendWrite(us); _us = us; _haveUs = true;
  float span = (float)(_maxUs - _minUs);
  float phys = ((float)us - (float)_minUs) / span * _travel - _trim;     // обратный пересчёт, чтобы angle() не врал
  _cur = _target = _start = _reverse ? _travel - phys : phys;
  _idleAt = millis();
}

void AlashServo::startMove(float a, uint32_t durationMs, Easing e) {
  _start = _cur; _target = a; _t0 = millis(); _dur = durationMs; _ease = e; _moving = true;
  apply();
}

void AlashServo::moveTo(float angle, uint32_t durationMs, Easing e) {
  if (!_attached) { _err = ERR_NOT_ATTACHED; return; }
  float a = clampAngle(angle);
  if (!finiteF(angle)) return;
  _sweeping = false;
  if (!_haveUs) { write(a); return; }              // положение неизвестно: едем сразу, плавность не от чего считать
  if (durationMs == 0) durationMs = travelTimeMs(a);
  if (durationMs == 0 || a == _cur) { write(a); return; }
  startMove(a, durationMs, e);
}

void AlashServo::stop() {
  _sweeping = false; _moving = false; _target = _cur; _idleAt = millis();
}

void AlashServo::sweep(float from, float to, uint32_t periodMs, Easing e) {
  if (!_attached) { _err = ERR_NOT_ATTACHED; return; }
  if (!finiteF(from) || !finiteF(to) || periodMs < 20) { _err = ERR_BAD_VALUE; return; }
  from = clampAngle(from); to = clampAngle(to);
  if (!_haveUs) write(from);
  _swA = from; _swB = to; _swPeriod = periodMs / 2;
  startMove(to, _swPeriod, e);
  _sweeping = true;
}

bool AlashServo::update() {
  if (!_attached) return false;
  uint32_t now = millis();
  if (_moving) {
    uint32_t el = now - _t0;
    if (el >= _dur) {
      _cur = _target; _moving = false;
      apply();
      _idleAt = now;
      if (_sweeping) {
        startMove(_target == _swB ? _swA : _swB, _swPeriod, _ease);
      } else if (_cb) {
        _cb();
      }
    } else {
      _cur = _start + (_target - _start) * easeCurve(_ease, (float)el / (float)_dur);
      apply();
    }
  } else if (_active && _idleMs > 0 && now - _idleAt >= _idleMs) {
    backendDetach();                               // отпустить сигнал: серво не гудит и не греется
    _active = false;
  }
  return _moving;
}

float AlashServo::progress() const {
  if (!_moving) return 1.0f;
  float p = (float)(millis() - _t0) / (float)_dur;
  return p > 1.0f ? 1.0f : p;
}

void AlashServo::setSpeed(float degreesPerSecond) {
  if (!finiteF(degreesPerSecond) || degreesPerSecond < 0) { _err = ERR_BAD_VALUE; return; }
  _speed = degreesPerSecond;
}

bool AlashServo::setLimits(float minAngle, float maxAngle) {
  if (!finiteF(minAngle) || !finiteF(maxAngle) || minAngle < 0 || maxAngle > _travel || minAngle >= maxAngle) { _err = ERR_BAD_VALUE; return false; }
  _lo = minAngle; _hi = maxAngle;
  return true;
}

uint32_t AlashServo::travelTimeMs(float toAngle) const {
  if (_speed <= 0) return 0;
  float d = toAngle > _cur ? toAngle - _cur : _cur - toAngle;
  uint32_t ms = (uint32_t)(d / _speed * 1000.0f + 0.5f);
  return ms;
}

// ---- Диагностика -------------------------------------------------------------------------------
const __FlashStringHelper *AlashServo::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NOT_ATTACHED:
      return T("Серво не подключено: сначала вызовите attach(пин).", "Серво қосылмаған: алдымен attach(пин) шақырыңыз.", "The servo is not attached: call attach(pin) first.");
    case ERR_BAD_PIN:
      return T("Неверный пин. Укажите номер пина, к которому подключён сигнальный провод серво (оранжевый или жёлтый).", "Пин қате. Серво сигнал сымы (сарғыш) қосылған пин нөмірін көрсетіңіз.", "Wrong pin. Give the pin number of the servo signal wire (orange or yellow).");
    case ERR_BAD_PULSE:
      return T("Длина импульса задана неверно: допустимо от 400 до 2700 мкс, максимум минимум на 100 мкс больше минимума. Типичные значения для SG90: 500 и 2400.", "Импульс ұзындығы қате: 400-2700 мкс, максимум минимумнан кемінде 100 мкс артық. SG90 үшін әдетте 500 және 2400.", "The pulse width is wrong: 400 to 2700 us, the maximum at least 100 us above the minimum. Typical SG90 values: 500 and 2400.");
    case ERR_RANGE:
      return T("Угол вне допустимого диапазона, команда выполнена до предела. Проверьте setLimits() и ход серво.", "Бұрыш рұқсат етілген аралықтан тыс, шекке дейін орындалды. setLimits() мен серво жүрісін тексеріңіз.", "The angle is out of range; the command went to the limit. Check setLimits() and the servo travel.");
    case ERR_NO_CHANNEL:
      return T("Нет свободного канала для серво: слишком много серво (на Uno до 12) либо на ESP32 заняты каналы ШИМ.", "Серво үшін бос арна жоқ: серво тым көп (Uno-да 12-ге дейін) немесе ESP32-де ШИМ арналары бос емес.", "No free channel for the servo: too many servos (up to 12 on Uno) or the ESP32 PWM channels are taken.");
    case ERR_BAD_VALUE:
      return T("Недопустимое значение: число не определено (NaN), отрицательная скорость или неверный ход/пределы.", "Жарамсыз мән: сан анықталмаған (NaN), теріс жылдамдық немесе жүріс/шектер қате.", "Invalid value: not a number (NaN), a negative speed or wrong travel/limits.");
  }
  return F("");
}

void AlashServo::diagnose(Print &out) {
  out.println(T("AlashServo: диагностика", "AlashServo: диагностика", "AlashServo: diagnostics"));
  if (!_attached) {
    out.println(T("Серво не подключено: вызовите attach(пин).", "Серво қосылмаған: attach(пин) шақырыңыз.", "The servo is not attached: call attach(pin)."));
    return;
  }
  out.print(T("Пин: ", "Пин: ", "Pin: ")); out.print((long)_pin);
  out.print(T("   импульс, мкс: ", "   импульс, мкс: ", "   pulse, us: ")); out.print((long)_minUs); out.print(F("..")); out.println((long)_maxUs);
  out.print(T("Ход, градусов: ", "Жүріс, градус: ", "Travel, degrees: ")); out.print(_travel, 1);
  out.print(T("   пределы: ", "   шектер: ", "   limits: ")); out.print(_lo, 1); out.print(F("..")); out.println(_hi, 1);
#if defined(ARDUINO_ARCH_ESP32)
  out.println(F("Output: ESP32 LEDC 50 Hz, 14 bit"));
#else
  out.println(F("Output: Servo.h"));
#endif
  out.print(T("Угол: ", "Бұрыш: ", "Angle: ")); out.print(_cur, 1);
  out.print(T("   скорость, град/с: ", "   жылдамдық, град/с: ", "   speed, deg/s: ")); out.print(_speed, 1);
  out.print(T("   реверс: ", "   кері: ", "   reverse: ")); out.println(_reverse ? T("да", "иә", "yes") : T("нет", "жоқ", "no"));
  out.println(T("Сейчас серво проедет в середину, потом на 45 градусов в обе стороны. Если стоит на месте или дёргается: ", "Қазір серво ортаға, содан екі жаққа 45 градусқа барады. Қозғалмаса немесе тартылса: ", "The servo will now go to the middle, then 45 degrees each way. If it stays put or jitters: "));
  out.println(T("  1) питание серво не от пина Arduino, а от отдельного источника 5 В (минимум 1 А на серво), земля общая;\n  2) сигнальный провод на нужном пине (на ESP32 подойдёт большинство GPIO, кроме только входных 34..39);\n  3) для SG90 попробуйте attach(пин, 500, 2400); для MG996R 500..2500.",
                "  1) серво қуаты Arduino пинінен емес, бөлек 5 В көзден (әр серваға кемінде 1 А), жер ортақ;\n  2) сигнал сымы дұрыс пинде (ESP32-де 34..39 тек кіріс пиндерінен басқа көпшілігі жарайды);\n  3) SG90 үшін attach(пин, 500, 2400); MG996R үшін 500..2500.",
                "  1) power the servo from a separate 5 V supply (at least 1 A per servo), not from the Arduino pin; grounds joined;\n  2) the signal wire on the right pin (on ESP32 most GPIOs work, except input-only 34..39);\n  3) for SG90 try attach(pin, 500, 2400); for MG996R 500..2500."));
  float mid = (_lo + _hi) * 0.5f, span = (_hi - _lo) * 0.25f;
  float seq[3] = {mid, mid - span, mid + span};
  for (uint8_t i = 0; i < 3; i++) {
    moveTo(seq[i], 800, SMOOTH);
    while (update()) delay(10);
    delay(200);
  }
  moveTo(mid, 800, SMOOTH);
  while (update()) delay(10);
}

// =================================================================================================
// Группа серво
// =================================================================================================
void AlashServoGroup::moveTo(const float *angles, uint32_t durationMs, AlashServo::Easing e) {
  if (durationMs == 0)
    for (uint8_t i = 0; i < _n; i++) { uint32_t t = _s[i].travelTimeMs(angles[i]); if (t > durationMs) durationMs = t; }
  for (uint8_t i = 0; i < _n; i++) _s[i].moveTo(angles[i], durationMs, e);
}

bool AlashServoGroup::update() {
  bool any = false;
  for (uint8_t i = 0; i < _n; i++) if (_s[i].update()) any = true;
  return any;
}

bool AlashServoGroup::moving() const {
  for (uint8_t i = 0; i < _n; i++) if (_s[i].moving()) return true;
  return false;
}

void AlashServoGroup::stop() { for (uint8_t i = 0; i < _n; i++) _s[i].stop(); }
void AlashServoGroup::center() { for (uint8_t i = 0; i < _n; i++) _s[i].center(); }
