// КОНФИГ

#define BT_PAUSE 5   // кнопка паузы
#define BT_RIGHT 3    // кнопка +
#define BT_LEFT 4     // кнопка -

#define GES_TOUT 900  // таймаут жестов
#define HOLD_TOUT 500 // таймаут удержания громкости
#define BTN_DELAY 50  // время удержания кнопки для "клика"


//БИБЛИОТЕКИ

#include "Wire.h"
#include "SparkFun_APDS9960.h"


//ПЕРЕМННЫЕ
 
#define APDS9960_INT 2
SparkFun_APDS9960 apds = SparkFun_APDS9960();
 
int isr_flag;
 

//СЕТАП

void setup() {
  Serial.begin(9600);
  attachInterrupt(0, interruptRoutine, FALLING);
 
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
 
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}


//ТАЙМЕР

uint32_t skip_tmr = 0;
bool checkTmr(uint32_t t) {
  return (millis() - skip_tmr >= t);
}
void rstTmr() {
  skip_tmr = millis();
}


//КНОПКИ

void click(byte pin) {
  press(pin);
  delay(BTN_DELAY);
  release(pin);
}
void press(byte pin) {
  pinMode(pin, OUTPUT);
}
void release(byte pin) {
  pinMode(pin, INPUT);
}


//ФЛАГИ

bool Rflag, Lflag;
void checkLeftHold() {
  if (Lflag) {
    release(BT_LEFT);
    Lflag = 0;
    Serial.println("Release BT_LEFT");
  }
}
void checkRightHold() {
  if (Rflag) {
    release(BT_RIGHT);
    Rflag = 0;
    Serial.println("Release BT_RIGHT");
  }
}
 

//ЛУП

void loop() {
  uint8_t data, data1;

  if (checkTmr(HOLD_TOUT) && (Rflag || Lflag)) {
    checkRightHold();
    checkLeftHold();
    rstTmr();
  }

  if( isr_flag == 1 ) {
    detachInterrupt(0);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(0, interruptRoutine, FALLING);
  }
}
 

//ЖЕСТЫ

void interruptRoutine() {
  isr_flag = 1;
}
 
void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        rstTmr();
        checkLeftHold();
        if (!Rflag) {
          Rflag = 1;
          press(BT_RIGHT);
          Serial.println("Press BT_RIGHT");
        }
        Serial.println("Vol Up");
        break;
      case DIR_DOWN:
        rstTmr();
        checkRightHold();
        if (!Lflag) {
          Lflag = 1;
          press(BT_LEFT);
          Serial.println("Press BT_LEFT");
        }
        Serial.println("Vol Down");
        break;
      case DIR_LEFT:
        if (!checkTmr(GES_TOUT)) return;
        rstTmr();
        click(BT_LEFT);
        Serial.println("Prev Track");
        break;
      case DIR_RIGHT:
        if (!checkTmr(GES_TOUT)) return;
        rstTmr();
        click(BT_RIGHT);
        Serial.println("Next Track");
        break;
      case DIR_NEAR:
        if (!checkTmr(GES_TOUT)) return;
        rstTmr();
        click(BT_PAUSE);
        Serial.println("Play/Pause");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}