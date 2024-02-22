//Основная функция отсчета времени
void clockTick() {
  if (clockTimer.isReady()) {
    SEC++;
    DEBUG(SEC);
    if (SEC > 59) {
      SEC = 0;
      MIN++;
    }
    if (MIN > 59) {
      MIN = 0;
      HOUR++;

      if (HOUR > 23) {
        HOUR = 0;
      }
      saveTimeVar();  //Сохраняем время в память каждый час
    }

    //Проверяем, не пора ли подать сигнал будильника
    if (alarmIsOn && HOUR == HOUR_A && MIN == MIN_A) {
      timeToBeep = true;
    }
    //Меняем флаг отображения точек
    pointFlag = !pointFlag;
  }
}

//Функция сигнала будильника
void alarmBeepBeep() {
  if (intervalBeepBeep.isReady()) {
    if (beepIsOn) {
      beepIsOn = false;  //Меняем состояние сигнала по таймеру
      DEBUG("BEEP BEEP");
      digitalWrite(PIN_BUZER, HIGH);
    } else {
      beepIsOn = true;  //Меняем состояние сигнала по таймеру
      digitalWrite(PIN_BUZER, LOW);
    }
  }
}

//Функция отображения температуры
void showTemprInDisplay() {
  pointFlag = false;
  display.point(pointFlag);
  int x = temperature / 10;  //Получаем десятки
  int y = temperature % 10;  //Получаем единицы
  display.display(0, x);     //(ячейка, ЦИФРА)
  display.display(1, y);
  //В третью и четвертую ячейку выводим знак градусов цельсия
  display.displayByte(2, _degree);  //(ячейка, байт)
  display.displayByte(3, _C);
}

//Функция демонстрации приветсвия
void showWelcomeText() {
  byte welcome_banner[] = { _empty, _empty, _H, _E, _L, _L, _O, _empty, _empty, _empty, _empty };
  display.runningString(welcome_banner, sizeof(welcome_banner), 200);  // 200 это время в миллисекундах!
}

//Функция демонстрации выключения будильника
void showOFFAlarm() {
  byte off_banner[] = { _O, _F, _F, _empty };
  display.runningString(off_banner, sizeof(off_banner), 200);  // 200 это время в миллисекундах!
}

//Функция демонстрации включения будильника
void showONAlarm() {
  byte on_banner[] = { _O, _N, _empty, _empty };
  display.runningString(on_banner, sizeof(on_banner), 200);  // 200 это время в миллисекундах!
}

//Функция загрузки переменных времени
void loadTimeVar() {
  // Если это первый запуск
  if (EEPROM.read(ADDR_INIT_KEY) != INIT_KEY) {
    EEPROM.write(ADDR_INIT_KEY, INIT_KEY);  //Записывем, что первый запуск был
    //Сохраняем переменные в память
    saveTimeVar();
  } else {
    //Загружаем переменные из памяти
    EEPROM.get(ADDR_HOUR, HOUR);      //Записывем HOUR часов
    EEPROM.get(ADDR_MIN, MIN);        //Записывем MIN часов
    EEPROM.get(ADDR_HOUR_A, HOUR_A);  //Записывем HOUR_A будильника
    EEPROM.get(ADDR_MIN_A, MIN_A);    //Записывем MIN_A будильника
  }
}

//Функция сохранения переменных времени
void saveTimeVar() {
  EEPROM.put(ADDR_HOUR, HOUR);              //Записывем HOUR часов
  EEPROM.put(ADDR_MIN, MIN);                //Записывем MIN часов
  EEPROM.put(ADDR_HOUR_A, HOUR_A);          //Записывем HOUR_A будильника
  EEPROM.put(ADDR_MIN_A, MIN_A);            //Записывем MIN_A будильника
  EEPROM.put(ADDR_ALARM_IS_ON, alarmIsOn);  //Записывем состояние будильника
}

void blinkMyClock() {
  //Мигаем цифрами в зависимости от режима
  if (blinkTimer.isReady()) {
    switch (settingsMode) {
      case 1:
        if (blinkFlag) {
          display.displayByte(0, _empty);  // 1 ячейка, пустой символ
          display.displayByte(1, _empty);  // 2 ячейка, пустой символ
        } else {
          display.displayClock(HOUR, MIN);  // выводим время часов
        }
        break;
      case 2:
        if (blinkFlag) {
          display.displayByte(2, _empty);  // 3 ячейка, пустой символ
          display.displayByte(3, _empty);  // 4 ячейка, пустой символ
        } else {
          display.displayClock(HOUR, MIN);  // выводим время часов
        }
        break;
      case 3:
        if (blinkFlag) {
          display.displayByte(0, _empty);  // 1 ячейка, пустой символ
          display.displayByte(1, _empty);  // 2 ячейка, пустой символ
        } else {
          display.displayClock(HOUR_A, MIN_A);  // выводим время часов
        }
        break;
      case 4:
        if (blinkFlag) {
          display.displayByte(2, _empty);  // 3 ячейка, пустой символ
          display.displayByte(3, _empty);  // 4 ячейка, пустой символ
        } else {
          display.displayClock(HOUR_A, MIN_A);  // выводим время часов
        }
        break;
    }
    blinkFlag = !blinkFlag;  //Инвертируем значение флага
  }
}
