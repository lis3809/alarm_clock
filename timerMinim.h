// мини-класс таймера

class timerMinim
{
  public:
    timerMinim(uint32_t interval);        // объявление таймера с указанием интервала
    void setInterval(uint32_t interval);  // установка интервала работы таймера
    boolean isReady();                    // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    void reset();                         // ручной сброс таймера на установленный интервал
    void stop();
    void start();

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
    bool _status = true;
};

timerMinim::timerMinim(uint32_t interval) {
  _interval = interval;
  _timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
  _interval = (interval == 0) ? 10 : interval;
}

void timerMinim::start() {
  _status = true;
  _timer = millis();
}

void timerMinim::stop() {
  _status = false;
}

// алгоритм таймера v2.0
boolean timerMinim::isReady() {
  uint32_t thisMls = millis();
  if (_status && thisMls - _timer >= _interval) {
    do {
      _timer += _interval;
      if (_timer < _interval) break;          // переполнение uint32_t
    } while (_timer < thisMls - _interval);  // защита от пропуска шага
    return true;
  } else {
    return false;
  }
}

void timerMinim::reset() {
  _timer = millis();
}
