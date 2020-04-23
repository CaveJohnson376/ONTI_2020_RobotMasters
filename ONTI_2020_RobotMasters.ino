#include <HCSR04.h>
#include <TroykaMQ.h>


// Пины и константы
#define distToFloor 13
#define echoPin 11
#define triggerPin 10
#define PIN_MQ135 A0            // Датчик газа
#define ventRelayPin A5         // Реле вентилятора
#define dCO2 20                 // Максимальное значение отклонения уровня газа
#define pinWaterSensor 12       // Датчик перелива
#define pumpDirPin1 4           // Первая помпа направление (пин моторшилда H1)
#define pumpSpeedPin1 5         // Первая помпа скорость (пин моторшилда E1)
#define pumpDirPin2 7           // Вторая помпа направление (пин моторшилда H2)
#define pumpSpeedPin2 6         // Вторая помпа скорость (пин моторшилда E1)

// Переменные
uint8_t pumpSpeed = 0;          // Скорости помп
bool pumpDir = HIGH;            // Направления помп
int CO2Opt = 0;                 // Среднее значение уровня газа
int CO2 = 0;

MQ135 mq135(PIN_MQ135);
UltraSonicDistanceSensor distSensor(triggerPin, echoPin);

void setup() {
  Serial.begin(9600);
  Serial.println("Code start.");
  pinMode(pumpDirPin1, OUTPUT);
  pinMode(pumpDirPin2, OUTPUT);
  pinMode(pumpSpeedPin1, OUTPUT);
  pinMode(pumpSpeedPin2, OUTPUT);
  pinMode(pinWaterSensor, INPUT);
  digitalWrite(pumpDirPin1, pumpDir);
  digitalWrite(pumpDirPin2, pumpDir);
  changeOutPumpState(false); // Выключаем в самом начале помпу в жилом боксе
  changeInPumpState(false);  // Выключаем в самом начале помпу в боксе с водой

  delay(60000);
  mq135.calibrate();
  Serial.print("CO2(0) = ");
  Serial.println(mq135.readCO2());
  gasCalibration();
}

void loop() {
  fillToWaterSensorLevel();
  CO2 = mq135.readCO2();
  if (CO2 < CO2Opt - dCO2 / 2) {
    changeAllWater();
  }
  if (CO2 > CO2Opt + dCO2 / 2) {
    Serial.println("Venting air...");
    while (CO2 > CO2Opt - dCO2 / 2) {
      digitalWrite(ventRelayPin, HIGH);
      CO2 = mq135.readCO2();
      delay(200);
    }
    digitalWrite(ventRelayPin, LOW);
    Serial.println("Finished venting.");
  }
}

void changeAllWater() { // Заменяет всю воду на новую
  Serial.println("Changing water...");
  removeAllWater();
  fillToWaterSensorLevel();
  Serial.println("Finished changing water.");
}

void removeAllWater() { // Убирает всю воду из бокса
  Serial.println("Removing water...");
  while (distSensor.measureDistanceCm() < distToFloor) {
    changeOutPumpState(true);
    delay(200);
  }
  changeOutPumpState(false);
  Serial.println("Finished removing water.");
}

void fillToWaterSensorLevel() { // Заполняет бокс водой до уровня датчика воды
  Serial.println("Filling water...");
  while (digitalRead(pinWaterSensor) == 0) {
    changeInPumpState(true);
    delay(20);
  }
  changeInPumpState(false);
  Serial.println("Finished filling water.");
}

void changeInPumpState(bool state) { // Включает/выключает насос в боксе с водой
  digitalWrite(pumpSpeedPin2, state);
}

void changeOutPumpState(bool state) { // Включавет/выключает насос в жилом боксе
  digitalWrite(pumpSpeedPin1, state);
}

void gasCalibration() { // Калибровка оптимального уровня углекислого газа
  Serial.println("Calibrating optimal gas levels...");
  fillToWaterSensorLevel()
  CO2Opt = mq135.readCO2();
  delay(10000);
  CO2Opt += mq135.readCO2();
  CO2Opt = CO2Opt / 2;
  Serial.print("Optimal Gas Level(const) = ");
  Serial.println(CO2Opt);
  Serial.println("Finished calibrating.");
  return ;
}
