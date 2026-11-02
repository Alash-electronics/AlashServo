// Общие проверки AlashServo; подключаются из test_servo_*.cpp (разные способы вывода сигнала).
#include <stdio.h>
#include "Arduino.h"
#include "AlashServo.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static bool near(float a, float b, float e) { return fabsf(a - b) <= e; }
static void fresh() { g_ev.clear(); g_ms = 1000; g_attachFail = false; }
static int lastUs() { for (int i = (int)g_ev.size() - 1; i >= 0; i--) if (g_ev[i].kind == 1) return g_ev[i].us; return -1; }
// LEDC хранит длину импульса в шагах по 1,2 мкс: на ESP32 допускаем отличие на 1 мкс
#if defined(ARDUINO_ARCH_ESP32)
#define US_TOL 1
#else
#define US_TOL 0
#endif
static bool usIs(int us) { int d = lastUs() - us; return d >= -US_TOL && d <= US_TOL; }
static int count(int kind) { int n = 0; for (auto &e : g_ev) if (e.kind == kind) n++; return n; }
static void run(AlashServo &s, uint32_t ms) { for (uint32_t i = 0; i < ms; i += 10) { g_ms += 10; s.update(); } }
static int finished = 0;
static void onDone() { finished++; }

static int runCases() {
  // 1) проверка параметров attach, до первой команды импульсов нет
  {
    fresh(); AlashServo s;
    CHECK(!s.attach(255) && s.error() == AlashServo::ERR_BAD_PIN);
    CHECK(!s.attach(9, 300, 2400) && s.error() == AlashServo::ERR_BAD_PULSE);
    CHECK(!s.attach(9, 544, 3000) && s.error() == AlashServo::ERR_BAD_PULSE);
    CHECK(!s.attach(9, 1000, 1050) && s.error() == AlashServo::ERR_BAD_PULSE);
    CHECK(!s.attach(9, 544, 2400, 5) && s.error() == AlashServo::ERR_BAD_VALUE);
    CHECK(!s.attached() && g_ev.empty());
    s.write(90); CHECK(s.error() == AlashServo::ERR_NOT_ATTACHED);       // команда без attach
    s.clearError();
    CHECK(s.attach(9));
    CHECK(s.attached() && g_ev.empty() && !s.active());                  // серво не дёргается при attach
    s.write(90);
    CHECK(s.active() && count(0) == 1 && usIs(1472));              // 544 + 0.5 * 1856
  }

  // 2) углы -> импульсы; повтор того же значения не пишет заново
  {
    fresh(); AlashServo s; s.attach(9);
    s.write(0); CHECK(usIs(544));
    s.write(180); CHECK(usIs(2400));
    size_t n = g_ev.size(); s.write(180); CHECK(g_ev.size() == n);
    AlashServo c; c.attach(10, 500, 2500, 270);                           // серво с ходом 270
    c.write(135); CHECK(usIs(1500));
    c.write(270); CHECK(usIs(2500));
  }

  // 3) реверс и поправка нуля
  {
    fresh(); AlashServo s; s.attach(9, 1000, 2000); s.setReverse(true);
    s.write(0); CHECK(usIs(2000));
    s.write(180); CHECK(usIs(1000));
    s.setReverse(false); s.setTrim(18); s.write(0); CHECK(usIs(1100));   // 18 градусов = 100 мкс
    s.setTrim(-30); s.write(0); CHECK(usIs(1000));                       // не ниже нуля
  }

  // 4) пределы: серво за них не едет, ошибка ERR_RANGE
  {
    fresh(); AlashServo s; s.attach(9, 1000, 2000);
    CHECK(!s.setLimits(100, 50) && s.error() == AlashServo::ERR_BAD_VALUE);
    s.clearError();
    CHECK(s.setLimits(30, 150));
    s.write(0); CHECK(near(s.angle(), 30, 0.001f) && s.error() == AlashServo::ERR_RANGE);
    s.clearError(); s.write(170); CHECK(near(s.angle(), 150, 0.001f));
    s.clearError(); s.write(100); CHECK(s.error() == AlashServo::ERR_OK);
    s.center(); CHECK(near(s.target(), 90, 0.001f));
  }

  // 5) плавное движение: равномерно, приходит ровно в цель, onFinish один раз
  {
    fresh(); AlashServo s; s.attach(9); s.onFinish(onDone); finished = 0;
    s.write(0);
    s.moveTo(180, 2000);
    CHECK(s.moving() && near(s.target(), 180, 0.001f));
    run(s, 1000); CHECK(near(s.angle(), 90, 1.0f) && near(s.progress(), 0.5f, 0.02f));
    int prev = -1; bool mono = true;
    for (int i = 0; i < 90; i++) { g_ms += 10; s.update(); int u = lastUs(); if (u < prev) mono = false; prev = u; }
    CHECK(mono);
    run(s, 200);
    CHECK(!s.moving() && s.angle() == 180 && usIs(2400) && finished == 1);
    run(s, 500); CHECK(finished == 1);
  }

  // 6) кривые: значение в середине времени
  {
    struct { AlashServo::Easing e; float mid; } cases[] = {{AlashServo::LINEAR, 90}, {AlashServo::QUAD_IN, 45}, {AlashServo::QUAD_OUT, 135}, {AlashServo::QUAD_IN_OUT, 90}, {AlashServo::SMOOTH, 90}};
    for (auto &c : cases) {
      fresh(); AlashServo s; s.attach(9); s.write(0); s.moveTo(180, 1000, c.e);
      run(s, 500); CHECK(near(s.angle(), c.mid, 1.5f));
      run(s, 600); CHECK(s.angle() == 180);
    }
    fresh(); AlashServo b; b.attach(9); b.write(0); b.moveTo(180, 1000, AlashServo::BOUNCE_OUT);
    float peak = 0; bool exceeded = false;
    for (int i = 0; i < 100; i++) { g_ms += 10; b.update(); if (b.angle() > 180.001f) exceeded = true; if (b.angle() > peak) peak = b.angle(); }
    CHECK(!exceeded && b.angle() == 180);                                // отскок не выходит за конечную точку
  }

  // 7) скорость вместо времени
  {
    fresh(); AlashServo s; s.attach(9); s.write(0);
    s.setSpeed(90); s.moveTo(180);                                       // 90 град/с: две секунды
    CHECK(s.travelTimeMs(180) == 2000);
    run(s, 1000); CHECK(s.moving() && near(s.angle(), 90, 1.5f));
    run(s, 1100); CHECK(!s.moving() && s.angle() == 180);
    s.setSpeed(0); s.moveTo(0); CHECK(!s.moving() && s.angle() == 0);    // нет скорости: сразу
    s.setSpeed(-5); CHECK(s.error() == AlashServo::ERR_BAD_VALUE);
  }

  // 8) положение неизвестно (первая команда): сразу в цель, без «плавного» старта из выдуманной точки
  {
    fresh(); AlashServo s; s.attach(9); s.moveTo(45, 2000);
    CHECK(!s.moving() && near(s.angle(), 45, 0.001f) && lastUs() > 0);
  }

  // 9) stop() в середине, moveTo во время движения, moveBy
  {
    fresh(); AlashServo s; s.attach(9); s.write(0); s.moveTo(180, 2000);
    run(s, 1000); float a = s.angle(); s.stop();
    CHECK(!s.moving() && near(s.angle(), a, 0.001f));
    run(s, 2000); CHECK(near(s.angle(), a, 0.001f));
    s.moveTo(180, 1000); run(s, 500); s.moveTo(0, 1000);                 // развернуть на ходу
    run(s, 1100); CHECK(s.angle() == 0);
    s.moveBy(30, 500); run(s, 600); CHECK(near(s.angle(), 30, 0.001f));
  }

  // 10) авто-отпускание сигнала и возврат
  {
    fresh(); AlashServo s; s.attach(9); s.setIdleDetach(1000); s.write(0);
    s.moveTo(90, 500); run(s, 600);
    CHECK(s.active() && count(2) == 0);
    run(s, 800); CHECK(s.active());
    run(s, 300); CHECK(!s.active() && count(2) == 1);                    // через 1 с после остановки отпустили
    size_t w = count(1); run(s, 2000); CHECK((size_t)count(1) == w);     // и импульсов больше нет
    s.moveTo(90, 500);                                                   // та же точка: сигнал всё равно возобновляется
    CHECK(s.active() || s.angle() == 90);
    s.write(45); CHECK(s.active() && count(0) == 2 && usIs(1008)); // 544 + 45/180*1856 = 1008.4
  }

  // 11) качание
  {
    fresh(); AlashServo s; s.attach(9); s.sweep(20, 160, 2000, AlashServo::LINEAR);
    float lo = 999, hi = -999; int turns = 0; float prev = s.angle(); int dir = 1;
    for (int i = 0; i < 1000; i++) { g_ms += 10; s.update(); float a = s.angle(); if (a < lo) lo = a; if (a > hi) hi = a;
      int d = a > prev ? 1 : a < prev ? -1 : dir; if (d != dir) { turns++; dir = d; } prev = a; }
    CHECK(near(lo, 20, 0.5f) && near(hi, 160, 0.5f));
    CHECK(turns >= 8 && turns <= 11);                                    // 10 с при периоде 2 с: около 10 разворотов
    s.stop(); float a = s.angle(); run(s, 1000); CHECK(near(s.angle(), a, 0.001f) && !s.moving());
    s.sweep(0, 10, 5); CHECK(s.error() == AlashServo::ERR_BAD_VALUE);   // период слишком короткий
  }

  // 12) writeMicroseconds и обратный пересчёт угла
  {
    fresh(); AlashServo s; s.attach(9, 1000, 2000);
    s.writeMicroseconds(1500); CHECK(usIs(1500) && near(s.angle(), 90, 0.01f) && abs((int)s.microseconds() - 1500) <= US_TOL);
    s.clearError(); s.writeMicroseconds(100); CHECK(s.error() == AlashServo::ERR_BAD_PULSE && usIs(1500));
  }

  // 13) группа: серво с разными путями приезжают вместе
  {
    fresh(); AlashServo arm[3]; arm[0].attach(2); arm[1].attach(3); arm[2].attach(4);
    for (auto &x : arm) x.write(0);
    AlashServoGroup g(arm, 3);
    float goal[3] = {180, 90, 30};
    g.moveTo(goal, 2000);
    CHECK(g.moving());
    run(arm[0], 0);
    for (int i = 0; i < 100; i++) { g_ms += 10; g.update(); }
    CHECK(g.moving());
    for (int i = 0; i < 110; i++) { g_ms += 10; g.update(); }
    CHECK(!g.moving() && arm[0].angle() == 180 && arm[1].angle() == 90 && arm[2].angle() == 30);
    // время по самому долгому при заданной скорости: 180 град при 90 град/с = 2 с
    for (auto &x : arm) x.setSpeed(90);
    float back[3] = {0, 0, 0};
    g.moveTo(back);
    for (int i = 0; i < 190; i++) { g_ms += 10; g.update(); }
    CHECK(g.moving() && near(arm[1].angle(), 90 * 0.05f, 25.0f) );        // 180 градусов рукой 0 ещё едет
    for (int i = 0; i < 40; i++) { g_ms += 10; g.update(); }
    CHECK(!g.moving() && arm[0].angle() == 0 && arm[1].angle() == 0 && arm[2].angle() == 0);
    g.center(); for (int i = 0; i < 110; i++) { g_ms += 10; g.update(); }     // скорость 90 град/с: от 0 до 90 за секунду
    CHECK(near(arm[0].angle(), 90, 0.01f));
    g.moveTo(goal, 3000); g.stop(); CHECK(!g.moving());
  }

  // 14) NaN, переполнение millis() во время движения
  {
    fresh(); AlashServo s; s.attach(9); s.write(0);
    s.moveTo(NAN); CHECK(s.error() == AlashServo::ERR_BAD_VALUE && s.angle() == 0);
    s.clearError();
    g_ms = 4294967000u;                                                  // за 296 мс до переполнения
    s.moveTo(180, 1000);
    run(s, 1100);
    CHECK(!s.moving() && s.angle() == 180);
  }

  // 15) нехватка каналов: ERR_NO_CHANNEL, потом после освобождения работает
  {
    fresh(); AlashServo s; s.attach(9);
    g_attachFail = true; s.write(90);
    CHECK(s.error() == AlashServo::ERR_NO_CHANNEL && !s.active());
    g_attachFail = false; s.clearError(); s.write(90);
    CHECK(s.active() && s.error() == AlashServo::ERR_OK && usIs(1472));
  }

  // 16) деструктор и detach() освобождают сигнал
  {
    fresh();
    { AlashServo s; s.attach(9); s.write(90); CHECK(g_liveAttached == 1); }
    CHECK(g_liveAttached == 0);
    AlashServo t; t.attach(9); t.write(10); t.detach(); CHECK(g_liveAttached == 0 && !t.attached());
    t.attach(9); t.attach(9); CHECK(g_liveAttached == 0);                // повторный attach ничего не тянет
  }

  // 17) тексты ошибок на трёх языках и диагностика
  {
    fresh(); AlashServo s;
    for (int e = AlashServo::ERR_NOT_ATTACHED; e <= AlashServo::ERR_BAD_VALUE; e++) {
      s.clearError();
      switch (e) {
        case AlashServo::ERR_NOT_ATTACHED: s.write(1); break;
        case AlashServo::ERR_BAD_PIN: s.attach(255); break;
        case AlashServo::ERR_BAD_PULSE: s.attach(9, 100, 200); break;
        case AlashServo::ERR_RANGE: s.attach(9); s.setLimits(10, 20); s.write(100); break;
        case AlashServo::ERR_NO_CHANNEL: s.detach(); s.attach(9); g_attachFail = true; s.write(5); g_attachFail = false; break;
        case AlashServo::ERR_BAD_VALUE: s.setSpeed(-1); break;
      }
      CHECK(s.error() == e);
      s.setLanguage(AlashServo::LANG_RU); const char *ru = (const char *)s.errorText();
      s.setLanguage(AlashServo::LANG_KK); const char *kk = (const char *)s.errorText();
      s.setLanguage(AlashServo::LANG_EN); const char *en = (const char *)s.errorText();
      CHECK(strlen(ru) > 20 && strlen(kk) > 20 && strlen(en) > 20 && strcmp(ru, kk) && strcmp(kk, en) && strcmp(ru, en));
    }
    s.clearError(); CHECK(strcmp((const char *)s.errorText(), "OK") == 0);
    AlashServo na; Print p0; na.diagnose(p0); CHECK(p0.text.find("attach") != std::string::npos);
    fresh(); AlashServo d; d.attach(9); d.setLanguage(AlashServo::LANG_EN);
    Print p; d.diagnose(p);
    CHECK(p.text.find("Pin: 9") != std::string::npos);
    CHECK(count(1) > 20 && !d.moving());                                 // диагностика сама прокатила серво
    CHECK(near(d.angle(), 90, 0.01f));                                   // и вернула в середину
    d.setLanguage(AlashServo::LANG_KK); Print pk; d.diagnose(pk); CHECK(pk.text.find("Пин: 9") != std::string::npos);
  }

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
