# AlashServo: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Серво, которое **едет плавно, а не прыгает**, и не останавливает программу: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Скорость, пределы, реверс, группы для руки-манипулятора, качание для радара. **Один код** для Arduino Uno, ESP8266 и ESP32, отдельные библиотеки не нужны. Без зависимостей.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashServo** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `attach(pin, minUs, maxUs, travel)` | Подключить; импульс для 0 и крайнего угла, ход (180 или 270) |
| `write(angle)` | Сразу в угол |
| `moveTo(angle, ms, easing)` | Плавно за время; ms = 0: по скорости |
| `moveBy(delta, ms, easing)` | Сдвинуть на столько-то градусов |
| `update()` | Вызывать в loop(); true, пока едет |
| `moving() / progress() / angle() / target()` | Состояние движения |
| `stop() / center()` | Остановить / в середину |
| `sweep(from, to, periodMs, easing)` | Качание туда-обратно |
| `setSpeed(deg/s)` | Скорость для moveTo без времени |
| `setLimits(min, max)` | Предел углов для защиты механики |
| `setReverse(bool) / setTrim(deg)` | Обратное направление / поправка нуля |
| `setIdleDetach(ms)` | Отпустить сигнал после остановки (не гудит) |
| `onFinish(fn)` | Вызвать функцию, когда доехали |
| `AlashServoGroup(servos, n) / moveTo(angles, ms)` | Группа: приезжают одновременно |
| `diagnose(Serial) / errorText() / setLanguage()` | Самопроверка и сообщения на ru/kk/en |

### Если что-то не работает
**Серво не двигается**

Серво нельзя питать от пина Arduino: нужен источник 5 В (минимум 1 А на серво), земля общая с платой. Проверьте пин сигнала и вызов `update()` в loop(). `diagnose(Serial)` прокатит серво и подскажет.

**Серво дёргается и гудит**

Обычно не хватает питания или импульс выходит за механику: уменьшите диапазон `attach(пин, 600, 2300)` или поставьте `setLimits()`. Для остановленного серво включите `setIdleDetach(1000)`.

**Крайние углы не совпадают с 0 и 180**

У разных серво разные импульсы. Для SG90 задайте `attach(пин, 500, 2400)`, для MG996R `500, 2500`. Если крайнее положение чуть не то, подправьте `setTrim()`. Для серво на 270 градусов последний аргумент `attach(пин, 500, 2500, 270)`.

**На Uno серво мешает ШИМ на пинах 9 и 10**

Встроенная библиотека Servo занимает Timer1, поэтому `analogWrite()` на пинах 9 и 10 не работает. Это ограничение платы, используйте другие пины для ШИМ. На ESP32 такого нет.

**На ESP32 серво не едет или ошибка канала**

Используйте обычные выходные GPIO (не 34..39, они только на вход). Если каналов LEDC не хватает, освободите ненужные ШИМ. Тексты ошибок покажет `errorText()`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

---

## Қазақша

**Секірмей, жылдам емес, баяу жүретін** серво, бағдарламаны тоқтатпайды: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Жылдамдық, шектер, кері бағыт, қол-манипулятор үшін топтар, радар үшін тербеліс. Arduino Uno, ESP8266 және ESP32 үшін **бір код**, бөлек кітапхана керек емес. Тәуелділіксіз.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashServo** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `attach(pin, minUs, maxUs, travel)` | Қосу |
| `write(angle)` | Бірден бұрышқа |
| `moveTo(angle, ms, easing)` | Баяу, уақыт бойынша |
| `moveBy(delta, ms, easing)` | Бірнеше градусқа жылжыту |
| `update()` | loop() ішінде шақыру |
| `moving() / progress() / angle() / target()` | Қозғалыс күйі |
| `stop() / center()` | Тоқтату / ортаға |
| `sweep(from, to, periodMs, easing)` | Алға-артқа тербеліс |
| `setSpeed(deg/s)` | Жылдамдық |
| `setLimits(min, max)` | Бұрыш шегі |
| `setReverse(bool) / setTrim(deg)` | Кері бағыт / нөл түзетуі |
| `setIdleDetach(ms)` | Тоқтағаннан кейін сигналды босату |
| `onFinish(fn)` | Жеткенде функцияны шақыру |
| `AlashServoGroup(servos, n) / moveTo(angles, ms)` | Топ: бір мезгілде жетеді |
| `diagnose(Serial) / errorText() / setLanguage()` | Өзін-өзі тексеру |

### Бірдеңе жұмыс істемесе
**Серво қозғалмайды**

Серваны Arduino пинінен қоректендіруге болмайды: 5 В көз (әр серваға кемінде 1 А), жер ортақ. Сигнал пинін және loop() ішіндегі `update()` тексеріңіз.

**Серво тартылады және ызыңдайды**

Әдетте қуат жеткіліксіз немесе импульс механикадан шығады: диапазонды азайтыңыз `attach(пин, 600, 2300)` немесе `setLimits()`. Тоқтаған серво үшін `setIdleDetach(1000)`.

**Шеткі бұрыштар 0 және 180-ге сәйкес келмейді**

Әртүрлі серволарда импульстер әртүрлі. SG90 үшін `attach(пин, 500, 2400)`, MG996R үшін `500, 2500`. `setTrim()` түзетеді. 270 градусты серво үшін соңғы аргумент 270.

**Uno-да серво 9 және 10 пиндеріндегі ШИМ-ге кедергі жасайды**

Кіріктірілген Servo кітапханасы Timer1-ді алады, сондықтан 9 және 10 пиндерінде `analogWrite()` жұмыс істемейді. Басқа пиндерді қолданыңыз. ESP32-де бұл жоқ.

**ESP32-де серво жүрмейді немесе арна қатесі**

Кәдімгі шығыс GPIO қолданыңыз (34..39 тек кіріс). LEDC арналары жетпесе, артық ШИМ-ді босатыңыз. `errorText()` қатені көрсетеді.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

---

## English

A servo that **moves smoothly instead of jumping** and never blocks the program: `servo.moveTo(120, 1500, AlashServo::SMOOTH)`. Speed, limits, reverse, groups for a robot arm, sweep for a radar. **One code** for Arduino Uno, ESP8266 and ESP32, no extra libraries. No dependencies.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashServo** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `attach(pin, minUs, maxUs, travel)` | Attach; pulse for 0 and the end angle, travel (180 or 270) |
| `write(angle)` | Straight to the angle |
| `moveTo(angle, ms, easing)` | Smoothly in a given time; ms = 0 uses the speed |
| `moveBy(delta, ms, easing)` | Move by some degrees |
| `update()` | Call in loop(); true while moving |
| `moving() / progress() / angle() / target()` | Motion state |
| `stop() / center()` | Stop / to the middle |
| `sweep(from, to, periodMs, easing)` | Back and forth |
| `setSpeed(deg/s)` | Speed for moveTo without time |
| `setLimits(min, max)` | Angle limits to protect the mechanics |
| `setReverse(bool) / setTrim(deg)` | Reverse / zero trim |
| `setIdleDetach(ms)` | Release the signal after stopping (no buzz) |
| `onFinish(fn)` | Call a function when arrived |
| `AlashServoGroup(servos, n) / moveTo(angles, ms)` | Group: arrive together |
| `diagnose(Serial) / errorText() / setLanguage()` | Self-check and messages in ru/kk/en |

### If something does not work
**The servo does not move**

Do not power a servo from an Arduino pin: use a 5 V supply (at least 1 A per servo) with a common ground. Check the signal pin and that `update()` is called in loop(). `diagnose(Serial)` runs the servo and gives hints.

**The servo jitters and buzzes**

Usually not enough power, or the pulse range goes beyond the mechanics: reduce it with `attach(pin, 600, 2300)` or set `setLimits()`. For a stopped servo use `setIdleDetach(1000)`.

**The end angles do not match 0 and 180**

Different servos need different pulses. SG90: `attach(pin, 500, 2400)`, MG996R: `500, 2500`. Fine-tune with `setTrim()`. For a 270 degree servo pass 270 as the last argument of `attach()`.

**On Uno the servo breaks PWM on pins 9 and 10**

The built-in Servo library takes Timer1, so `analogWrite()` does not work on pins 9 and 10. This is a board limit, use other pins for PWM. ESP32 has no such issue.

**On ESP32 the servo does not move or a channel error appears**

Use normal output GPIOs (not 34..39, those are input-only). If LEDC channels run out, free unused PWM. `errorText()` shows the error.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Smooth`, `Sweep`, `Arm`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashServo
