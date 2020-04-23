#include <TroykaMQ.h>
#include <iarduino_HC_SR04.h>


// Пины и константы

#define PIN_MQ135 A0                 // Датчик газа
#define dCO2 20                      // Максимальное значение отклонения уровня газа
#define pinWDat 12                   // Датчик перелива
#define pumpdir1 4                   // Первая помпа направление (пин моторшилда H1)
#define pumpspd1 5                   // Первая помпа скорость (пин моторшилда E1)
#define pumpdir2 7                   // Вторая помпа направление (пин моторшилда H2)
#define pumpspd2 6                   // Вторая помпа скорость (пин моторшилда E1)

// Переменные
uint8_t pumpspd = 0;                 // Скорости помп
bool    pumpdir = HIGH;              // Направления помп
int     CO2     = 0;                 // Среднее значение уровня газа

MQ135 mq135(PIN_MQ135);

void setup(){
  Serial.begin(9600);

  pinMode(pumpdir1, OUTPUT); digitalWrite(pumpdir1, pumpdir);          // Конфигурируем вывод pinH1 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pumpdir2, OUTPUT); digitalWrite(pumpdir2, pumpdir);          // Конфигурируем вывод pinE1 как выход и устанавливаем на нём уровень логического «0»
  pinMode(pumpspd1, OUTPUT); digitalWrite(pumpspd1, 0);
  pinmode(pumpspd2, OUTPUT); digitalWrite(pumpspd2, 0);
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

void colibration(){ // Калибровка
  digitalWrite(pumpspd1, HIGH); // Начинаем заливать воду
  while(digitalRead(pinWDat)==0){
    delay(1);
  }
  digitalWrite(pumpspd1, LOW);
  delay(10000);
  CO2=mq135.readCO2()/2;
  Serial.print("CO2(const) = ");
  Serial.println(CO2);
  return ;
}
