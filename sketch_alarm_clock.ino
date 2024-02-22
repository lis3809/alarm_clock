//============== Отладка / Работа ===========================
#define DEBUG_UART 1  // 1 - отладка, 0 - работа

//================= Константы ===============================
#define CLK 5        //Пин CLK
#define DIO 4        //Пин DIO
#define PIN_MOD 2    //Пин кнопки переключения режимов
#define PIN_SET 3    //Пин кнопки изменения параметров
#define PIN_TEMPR 6  //Пин датчика температуры
#define PIN_BUZER 8  //Пин пьезодинамика

#define ADDR_INIT_KEY 20      // адрес ячейки ключа
#define INIT_KEY 5           // Значение ключа для первого запуска
#define ADDR_HOUR 50          // адрес ячейки для HOUR часов
#define ADDR_MIN 100          // адрес ячейки для MIN часов
#define ADDR_HOUR_A 150       // адрес ячейки для HOUR_A будильника
#define ADDR_MIN_A 200        // адрес ячейки для MIN_A будильника
#define ADDR_ALARM_IS_ON 250  // адрес ячейки для флага состояния будильника

//==============Библиотеки===============================
#include "GyverTM1637.h"
#include "DHT.h"
#include "buttonMinim.h"
#include "timerMinim.h"
#include <EEPROM.h>

//================= Переменные ===============================
//Объявляем переменные для часов и будильника и присваиваем им начальные значения
byte HOUR = 13;  //Начальное значение часов
byte MIN = 21;   //Начальное значение минут
byte SEC = 0;    //Начальное значение секунд

byte HOUR_A = 13;  //Начальное значение часов будильника
byte MIN_A = 22;   //Начальное значение минут будильника

int temperature = 25;  //Начальное значение температуры
                       //Переменная режимов настройки
int settingsMode = 0;  //0 - нормальный режим работы часов
                       //1 - режим настройки HOUR часов
                       //2 - режим настройки MIN часов
                       //3 - режим настройки HOUR_A будильника
                       //4 - режим настройки MIN_A будильника

bool pointFlag = true;         //Флаг включения и отключения точек
bool timeToShowTempr = false;  //Флаг для отображения температуры
bool timeToBeep = false;       //Флаг для подачи звукового сигнала будильника
bool beepIsOn = false;         //Флаг для настройки длины сигнала будильника
bool alarmIsOn = true;         //Флаг вкл./выкл. будильника
bool blinkFlag = false;        //Флаг для мигания цифрами в режиме настроек

//====================== РАЗЛИЧНЫЕ ОБЪЕКТЫ ===========
GyverTM1637 display(CLK, DIO);
DHT dht(PIN_TEMPR, DHT11);

buttonMinim btnMode(PIN_MOD);  //Для кнопки переключения режимов
buttonMinim btnSet(PIN_SET);   //Для кнопки изменения параметров

timerMinim clockTimer(1000);       //Главный таймер часов - 1 секунда
timerMinim checkSensor(10000);     //Таймер проверки датчика температуры
timerMinim showTemprTimer(2000);   //Таймер для отображения температуры в течении 2 сек
timerMinim intervalBeepBeep(400);  //Таймер для настройки длины сигнала будильника
timerMinim blinkTimer(400);        //Таймер для мигания числами в режиме настроек


//======================== МАКРО-ЗАМЕНЫ =========================
#if (DEBUG_UART == 1)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

void setup() {
#if (DEBUG_UART == 1)
  Serial.begin(9600);
  DEBUG("start");
#endif

  //Устанавливаем режим работы кнопок
  pinMode(PIN_MOD, INPUT_PULLUP);  //Режим пина кнопки переключения режимов
  pinMode(PIN_SET, INPUT_PULLUP);  //Режим пина кнопки изменения параметров
  pinMode(PIN_TEMPR, INPUT);       //Режим пина датчика температуры
  pinMode(PIN_BUZER, OUTPUT);      //Режим пина пьезодинамика
  digitalWrite(PIN_BUZER, HIGH);   //Устанавливаем высокий уровень сигнала для динамика

  //Загружаем ранее сохраненные данные
  loadTimeVar();

  //Таймером показа температуры. Будем управлять вручную
  showTemprTimer.stop();

  //Запускаем работу датчика
  dht.begin();

  //Очищаем дисплей
  display.clear();
  //Устанавливаем яркость дисплея
  display.brightness(2);  //яркость дисплея от 0 до 7
  //Отображаем приветствие
  showWelcomeText();
}

void loop() {

  //Главная функция отсчета времени
  clockTick();

  //Что отображать? Время или температуру
  if (timeToShowTempr && !timeToBeep) {
    showTemprInDisplay();
    //Если показывать температуру достаточно, меняем флаг и выключаем таймер
    if (showTemprTimer.isReady()) {
      showTemprTimer.stop();
      timeToShowTempr = false;
    }
  } else {
    display.point(pointFlag);
    display.displayClock(HOUR, MIN);  // выводим время
  }

  //Если пришло время, проверяем датчик
  if (checkSensor.isReady()) {
    temperature = dht.readTemperature() - 4;  //Небольшая поправка температуры из-за нагрева ардуино
    timeToShowTempr = true;
    showTemprTimer.start();
  }

  //Если пришло время, подаем сигнал будильника
  if (timeToBeep) {
    alarmBeepBeep();
  }

  //Если долго зажата кнопка Set
  if (btnSet.holded()) {
    pointFlag = false;
    display.point(pointFlag);
    alarmIsOn = !alarmIsOn;
    if (alarmIsOn) {
      showONAlarm();
      saveTimeVar();
    } else {
      showOFFAlarm();
      timeToBeep = false;
      beepIsOn = false;  //Меняем состояние сигнала по таймеру
      digitalWrite(PIN_BUZER, HIGH);
    }
  }

  //Если долго зажата кнопка Mod
  if (btnMode.holded()) {
    //Очищаем дисплей
    display.clear();
    //Включаем отображение точек
    pointFlag = true;
    display.point(pointFlag);
    display.displayClockScroll(HOUR, MIN, 70);  // выводим время часов

    settingsMode = 1;  //Устанавливаем режим настройки HOUR часов


    DEBUG("SETTINGS");
    while (settingsMode != 0) {
      //Если нажата кнопка Mode - переходим в следующий режим
      if (btnMode.clicked()) {
        settingsMode++;

        if (settingsMode == 1 || settingsMode == 2) {
          //Очищаем дисплей
          display.clear();
          display.displayClockScroll(HOUR, MIN, 70);  // выводим время часов
        }

        if (settingsMode == 3 || settingsMode == 4) {
          //Очищаем дисплей
          display.clear();
          display.displayClockScroll(HOUR_A, MIN_A, 70);  // выводим время будильника
        }

        if (settingsMode > 4) {
          settingsMode = 0;  //Устанавливаем основной режим работы
          saveTimeVar();     //Сохраняем настроенное время
          return;            //Выходим из метода
        }
      }

      //Если нажата кнопка Set - меняем значение переменных в зависимости от режима настройки
      if (btnSet.clicked()) {
        switch (settingsMode) {
          case 1:
            HOUR++;
            if (HOUR > 23) {
              HOUR = 0;
            }
            display.displayClockScroll(HOUR, MIN, 70);  // выводим время часов
            break;
          case 2:
            MIN++;
            if (MIN > 59) {
              MIN = 0;
            }
            display.displayClockScroll(HOUR, MIN, 70);  // выводим время часов
            break;
          case 3:
            HOUR_A++;
            if (HOUR_A > 23) {
              HOUR_A = 0;
            }
            display.displayClockScroll(HOUR_A, MIN_A, 70);  // выводим время часов
            break;
          case 4:
            MIN_A++;
            if (MIN_A > 59) {
              MIN_A = 0;
            }
            display.displayClockScroll(HOUR_A, MIN_A, 70);  // выводим время часов
            break;
        }
      }

      //Мигаем цифрами в зависимости от режима настройки
      blinkMyClock();
    }
  }
}
