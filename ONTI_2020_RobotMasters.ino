#include <TroykaMQ.h>

#include <iarduino_HC_SR04.h>


// имя для пина, к которому подключен датчик
#define PIN_MQ135         A0

const int     dCO2    = 20;
const uint8_t pinWDat = 12;  
const uint8_t pinH1   = 4;                                   // Создаём константу указывая номер вывода H1 MotorShield (он управляет направлением 1 мотора)
const uint8_t pinE1   = 5;                                   // Создаём константу указывая номер вывода E1 MotorShield (он управляет скоростью    1 мотора)
const uint8_t pinE2   = 6;                                   // Создаём константу указывая номер вывода E2 MotorShield (он управляет скоростью    2 мотора)
const uint8_t pinH2   = 7;                                   // Создаём константу указывая номер вывода H2 MotorShield (он управляет направлением 2 мотора)
      uint8_t mSpeed  = 0;                                 // Создаём переменную для хранения скорости    моторов
      bool    mDirect = HIGH;                                // Создаём переменную для хранения направления моторов
      int     CO2     = 0;

MQ135 mq135(PIN_MQ135);

void setup(){
  Serial.begin(9600);
  pinMode(pinH1, OUTPUT); digitalWrite(pinH1, mDirect);          // Конфигурируем вывод pinH1 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pinE1, OUTPUT); digitalWrite(pinE1, LOW);          // Конфигурируем вывод pinE1 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pinE2, OUTPUT); digitalWrite(pinE2, LOW);          // Конфигурируем вывод pinE2 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pinH2, OUTPUT); digitalWrite(pinH2, mDirect);          // Конфигурируем вывод pinH2 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pinWDat, INPUT);
  delay(60000);
  mq135.calibrate();
  Serial.print("CO2(0) = ");
  Serial.println(mq135.readCO2());
  colibration();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void colibration(){
  digitalWrite(pinE1, HIGH);
  while(digitalRead(pinWDat)==0){
    delay(1);
  }
  digitalWrite(pinE1, LOW);
  delay(10000);
  CO2=mq135.readCO2()/2;
  Serial.print("CO2(const) = ");
  Serial.println(CO2);
  return ;
}
