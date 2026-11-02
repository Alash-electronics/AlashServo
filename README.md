# AlashServo

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Серво, которое **едет плавно, а не прыгает**, и не останавливает программу: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Скорость, пределы, реверс, группы для руки-манипулятора, качание для радара. **Один код** для Arduino Uno, ESP8266 и ESP32, отдельные библиотеки не нужны. Без зависимостей.

```cpp
#include <AlashServo.h>
AlashServo servo;

void setup() {
  servo.attach(9);                    // пин сигнального провода
  servo.write(0);                     // первое положение сразу
  servo.moveTo(150, 2000);            // плавно за 2 секунды
}
void loop() {
  servo.update();                     // в каждом проходе loop()
}
```

### Что умеет
- **`moveTo(угол, время, кривая)`:** плавное движение без `delay()`; программа продолжает работать.
- **Кривые:** `LINEAR`, `QUAD_IN`, `QUAD_OUT`, `QUAD_IN_OUT`, `SMOOTH`, `BOUNCE_OUT`.
- **Скорость:** `setSpeed(град/с)`, тогда `moveTo(угол)` сам считает время.
- **Защита механики:** `setLimits(мин, макс)`, серво за пределы не поедет; `setReverse`, `setTrim`.
- **Группа:** `AlashServoGroup` двигает руку-манипулятор так, что все суставы приезжают одновременно.
- **`sweep()`:** качание для радара или головы робота.
- **Не гудит:** `setIdleDetach(мс)` отпускает сигнал после остановки.
- **Серво не дёргается при подключении:** импульсы идут только после первой команды.
- **ESP32 без доп. библиотек:** свой вывод через аппаратный ШИМ LEDC, ядра 2.x и 3.x.
- **Видно, что происходит:** `diagnose(Serial)` прокатывает серво и подсказывает про питание и пины на трёх языках.
- Работает на **Arduino AVR, ESP8266, ESP32**.

### Чем отличается от других библиотек
Мы изучили встроенную Servo, ServoEasing (ArminJo), ESP32Servo, AsyncServoLib и VarSpeedServo:
- Встроенная Servo только прыгает в угол и не знает ESP32; у нас плавность и один код для всех плат.
- ServoEasing мощная, но под GPL-3.0 (форкнуть в MIT нельзя), тянет макросы и прерывания таймеров; у нас простая опросная модель без прерываний.
- ESP32Servo нужно ставить отдельно и она ничего не знает про плавность; у нас LEDC встроен.
- Ни в одной нет сообщений и самопроверки на русском и казахском.

### Примеры
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

### Тесты
Без железа проверены: пересчёт угла в импульс, реверс и поправка, пределы, плавное движение и кривые, скорость, автоотпускание, качание, группа, переполнение `millis()`, ошибки и диагностика. Все три способа вывода (Servo.h, LEDC ядра 3.x и 2.x) прогоняются на одном наборе проверок: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**Секірмей, жылдам емес, баяу жүретін** серво, бағдарламаны тоқтатпайды: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Жылдамдық, шектер, кері бағыт, қол-манипулятор үшін топтар, радар үшін тербеліс. Arduino Uno, ESP8266 және ESP32 үшін **бір код**, бөлек кітапхана керек емес. Тәуелділіксіз.

```cpp
#include <AlashServo.h>
AlashServo servo;

void setup() {
  servo.attach(9);                    // пин сигнального провода
  servo.write(0);                     // первое положение сразу
  servo.moveTo(150, 2000);            // плавно за 2 секунды
}
void loop() {
  servo.update();                     // в каждом проходе loop()
}
```

### Мүмкіндіктері
- **`moveTo(бұрыш, уақыт, қисық)`:** `delay()` жоқ баяу қозғалыс; бағдарлама жұмысын жалғастырады.
- **Қисықтар:** `LINEAR`, `QUAD_IN`, `QUAD_OUT`, `QUAD_IN_OUT`, `SMOOTH`, `BOUNCE_OUT`.
- **Жылдамдық:** `setSpeed(град/с)`, сонда `moveTo(бұрыш)` уақытты өзі есептейді.
- **Механиканы қорғау:** `setLimits(мин, макс)`; `setReverse`, `setTrim`.
- **Топ:** `AlashServoGroup` қол-манипулятордың барлық буынын бірге жеткізеді.
- **`sweep()`:** радар немесе робот басы үшін тербеліс.
- **Ызыңдамайды:** `setIdleDetach(мс)` тоқтағаннан кейін сигналды босатады.
- **Қосқанда серво тартылмайды:** импульстер бірінші команданың алдында жүрмейді.
- **ESP32-де қосымша кітапханасыз:** LEDC аппараттық ШИМ арқылы, 2.x және 3.x ядролары.
- **Не болып жатқаны көрінеді:** `diagnose(Serial)` серваны жүргізіп, қуат пен пиндер туралы үш тілде кеңес береді.
- **Arduino AVR, ESP8266, ESP32** тақталарында жұмыс істейді.

### Басқа кітапханалардан айырмашылығы
Кіріктірілген Servo, ServoEasing (ArminJo), ESP32Servo, AsyncServoLib және VarSpeedServo қарадық:
- Кіріктірілген Servo бұрышқа секіреді, ESP32-ні білмейді; бізде баяу қозғалыс және барлық тақтаға бір код.
- ServoEasing күшті, бірақ GPL-3.0 (MIT-ке форк жасау мүмкін емес), макростар мен таймер үзілістері; бізде қарапайым сұрау үлгісі.
- ESP32Servo бөлек орнатылады; бізде LEDC кіріктірілген.
- Ешқайсысында орысша және қазақша хабарлар мен өзін-өзі тексеру жоқ.

### Мысалдар
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

### Тесттер
Құрылғысыз тексерілген: бұрыштан импульске аудару, кері бағыт, шектер, баяу қозғалыс пен қисықтар, жылдамдық, авто-босату, тербеліс, топ, `millis()` толуы, қателер. Шығарудың үш тәсілі (Servo.h, LEDC 3.x және 2.x) бір тексеру жиынымен: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A servo that **moves smoothly instead of jumping** and never blocks the program: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Speed, limits, reverse, groups for a robot arm, sweep for a radar. **One code** for Arduino Uno, ESP8266 and ESP32, no extra libraries. No dependencies.

```cpp
#include <AlashServo.h>
AlashServo servo;

void setup() {
  servo.attach(9);                    // пин сигнального провода
  servo.write(0);                     // первое положение сразу
  servo.moveTo(150, 2000);            // плавно за 2 секунды
}
void loop() {
  servo.update();                     // в каждом проходе loop()
}
```

### Features
- **`moveTo(angle, time, easing)`:** smooth motion without `delay()`; the program keeps running.
- **Curves:** `LINEAR`, `QUAD_IN`, `QUAD_OUT`, `QUAD_IN_OUT`, `SMOOTH`, `BOUNCE_OUT`.
- **Speed:** `setSpeed(deg/s)`, then `moveTo(angle)` works out the time itself.
- **Protects the mechanics:** `setLimits(min, max)`, the servo never goes beyond; `setReverse`, `setTrim`.
- **Group:** `AlashServoGroup` moves a robot arm so that all joints arrive together.
- **`sweep()`:** back and forth for a radar or a robot head.
- **No buzzing:** `setIdleDetach(ms)` releases the signal after stopping.
- **No jerk on attach:** pulses start only with the first command.
- **ESP32 without extra libraries:** drives the LEDC hardware PWM itself, cores 2.x and 3.x.
- **See what happens:** `diagnose(Serial)` runs the servo and gives tips on power and pins in three languages.
- Works on **Arduino AVR, ESP8266, ESP32**.

### How it differs from other libraries
We studied the built-in Servo, ServoEasing (ArminJo), ESP32Servo, AsyncServoLib and VarSpeedServo:
- The built-in Servo only jumps to an angle and does not know ESP32; ours is smooth and one code for every board.
- ServoEasing is powerful but GPL-3.0 (cannot be forked into MIT) and relies on macros and timer interrupts; ours is a simple polling model without interrupts.
- ESP32Servo must be installed separately and knows nothing about smoothness; ours has LEDC built in.
- None has messages and a self-check in Russian and Kazakh.

### Examples
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

### Tests
Checked without hardware: angle to pulse conversion, reverse and trim, limits, smooth motion and curves, speed, auto release, sweep, group, `millis()` overflow, errors and diagnostics. All three output paths (Servo.h, LEDC cores 3.x and 2.x) run the same set of checks: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
