#include <HCSR04.h>
#include <TroykaMQ.h>



// Пины и константы

#define echoPin 11
#define triggerPin 10
#define PIN_MQ135 A0                 // Датчик газа
#define dCO2 20                      // Максимальное значение отклонения уровня газа
#define pinWaterSensor 12                   // Датчик перелива
#define pumpDirPin1 4                   // Первая помпа направление (пин моторшилда H1)
#define pumpSpeedPin1 5                   // Первая помпа скорость (пин моторшилда E1)
#define pumpDirPin2 7                   // Вторая помпа направление (пин моторшилда H2)
#define pumpSpeedPin2 6                   // Вторая помпа скорость (пин моторшилда E1)

// Переменные
uint8_t pumpSpeed = 0;                 // Скорости помп
bool    pumpDir = HIGH;              // Направления помп
int     CO2Opt     = 0;// Среднее значение уровня газа
int     CO2 = 0;
MQ135 mq135(PIN_MQ135);
UltraSonicDistanceSensor distSensor(triggerPin, echoPin);

void setup(){
  Serial.begin(9600);

  pinMode(pumpDirPin1, OUTPUT);
  digitalWrite(pumpDirPin1, pumpDir);
  pinMode(pumpDirPin2, OUTPUT); digitalWrite(pumpDirPin2, pumpDir);
  pinMode(pumpSpeedPin1, OUTPUT); changeOutPumpState(false); // Выключаем в самом начале помпу в жилом боксе
  pinMode(pumpSpeedPin2, OUTPUT); changeInPumpState(false); // Выключаем в самом начале помпу в боксе с водой
  pinMode(pinWaterSensor, INPUT);
  delay(60000);
  mq135.calibrate();
  Serial.print("CO2(0) = ");
  Serial.println(mq135.readCO2());
  gasCalibration();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void changeInPumpState(bool state) { // Включает/выключает насос в боксе с водой
  digitalWrite(pumpSpeedPin2, state);
}

void changeOutPumpState(bool state) { // Включавет/выключает насос в жилом боксе
  digitalWrite(pumpSpeedPin1, state);
}

void gasCalibration(){ // Калибровка
  changeInPumpState(true); // Начинаем заливать воду
  while(digitalRead(pinWaterSensor) == 0){
    delay(1);
  }
  changeInPumpState(false);
  CO2Opt = mq135.readCO2();
  delay(10000);
  CO2Opt += mq135.readCO2();
  CO2Opt = CO2Opt / 2;
  Serial.print("Optimal Gas Level(const) = ");
  Serial.println(CO2Opt);
  return ;
}
