#include <TroykaMQ.h>
#include <NewPing.h>
#include <ARpcDevice.h>

// Всё что связанно с Alterozoom
const char *deviceName="Robot_Masters_Habitat_Device";//имя устройства
const ARpcUuid deviceId("{835a5823-b43f-4b9c-9acf-04e758783720}");//идентификатор устройства
const char *sensorsDef="<sensors>"
"<sensor name=\"co2_sensor\" type=\"u16_sv\"/>"// Датчик углекислого газа
"<sensor name=\"ultrasonic\" type=\"f32_sv\"/>"// Ультразвук
"</sensors>";

class WriteCallback
    :public ARpcIWriteCallback
{
public:
    virtual void writeData(const char *data,unsigned long sz)
    {
        Serial.write((uint8_t *)(data),sz);
    }
    virtual void writeStr(const char *str)
    {
        Serial.print(str);
    }
    virtual void writeStr(const __FlashStringHelper *str)
    {
        Serial.print(str);
    }
}wcb;

ARpcDevice dev(300,&wcb,&deviceId,deviceName);

// Пины и константы
#define timeBetweenLoggingSensors 200
#define distToFloor 13
#define echoPin 11
#define triggerPin 10
#define PIN_MQ135 A0            // Датчик газа
#define ventRelayPin A5         // Реле вентилятора
#define dCO2 20                 // Максимальное значение отклонения уровня газа
#define pinWaterSensor 12       // Датчик перелива
#define pumpDirPin1 4           // Первая помпа направление (пин моторшилда H1)
#define pumpSpeedPin1 5         // Первая помпа скорость (пин моторшилда E1), в боксе с водой
#define pumpDirPin2 7           // Вторая помпа направление (пин моторшилда H2)
#define pumpSpeedPin2 6         // Вторая помпа скорость (пин моторшилда E1), в жилом боксе

// Переменные
uint8_t pumpSpeed = 0;          // Скорости помп
bool pumpDir = HIGH;            // Направления помп
int CO2Opt = 0;                 // Среднее значение уровня газа
int CO2 = 0;
unsigned long long lastMeasurementTime = 0;

MQ135 mq135(PIN_MQ135);
NewPing sonar(triggerPin, echoPin, 50);

void setup() {
  Serial.begin(9600);
  dev.disp().setSensors(sensorsDef);
  dev.resetStream();
  Serial.println("Code start.");
  pinMode(pumpDirPin1, OUTPUT);
  pinMode(pumpDirPin2, OUTPUT);
  pinMode(pumpSpeedPin1, OUTPUT);
  pinMode(pumpSpeedPin2, OUTPUT);
  pinMode(pinWaterSensor, INPUT_PULLUP);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(pumpDirPin1, pumpDir);
  digitalWrite(pumpDirPin2, pumpDir);
  changeOutPumpState(false); // Выключаем в самом начале помпу в жилом боксе
  changeInPumpState(false);  // Выключаем в самом начале помпу в боксе с водой

  delay(60000);
  mq135.calibrate();
  Serial.print("CO2(0) = ");
  Serial.println(mq135.readCO2());
  removeAllWater();
  gasCalibration();
}

void loop() {
  fillToWaterSensorLevel();
  CO2 = mq135.readCO2();
  logSensorMeasurements();
  Serial.print("CO2: ");
  Serial.print(CO2);
  Serial.println("ppm");
  if (CO2 < CO2Opt - dCO2 / 2) {
    changeAllWater();
  }
  if (CO2 > CO2Opt + dCO2 / 2) {
    Serial.println("Venting air...");
    while (CO2 > CO2Opt - dCO2 / 2) {
      logSensorMeasurements();
      digitalWrite(ventRelayPin, HIGH);
      CO2 = mq135.readCO2();
      delay(200);
    }
    digitalWrite(ventRelayPin, LOW);
    Serial.println("Finished venting.");
  }
  delay(1000);
}

void changeAllWater() { // Заменяет всю воду на новую
  Serial.println("Changing water...");
  removeAllWater();
  fillToWaterSensorLevel();
  Serial.println("Finished changing water.");
}

void removeAllWater() { // Убирает всю воду из бокса
  Serial.println("Removing water...");
  while (sonar.ping_cm() < distToFloor) { // расстояние до воды будет увеличиваться, т.к. вода будет выкачиваться
    logSensorMeasurements();
    Serial.print("Distance: ");
    Serial.print(sonar.ping_cm());
    Serial.println("cm");
    changeOutPumpState(true);
    delay(200);
  }
  changeOutPumpState(false);
  Serial.println("Finished removing water.");
}

void fillToWaterSensorLevel() { // Заполняет бокс водой до уровня датчика воды
  Serial.println("Filling water...");
  while (digitalRead(pinWaterSensor) == 1) { // датчик воды возвращает 1 когда нету воды
    logSensorMeasurements();
    changeInPumpState(true);
    delay(20);
  }
  changeInPumpState(false);
  Serial.println("Finished filling water.");
}

void changeOutPumpState(bool state) { // Включает/выключает насос в жилом боксе
  digitalWrite(pumpSpeedPin2, state);
}

void changeInPumpState(bool state) { // Включает/выключает насос в боксе с водой
  digitalWrite(pumpSpeedPin1, state);
}

void logSensorMeasurements() {
  if (millis() - lastMeasurementTime >= timeBetweenLoggingSensors) {
    char CO2Data[255];
    char ultrasonicData[255];
    String(mq135.readCO2()).toCharArray(CO2Data, 255);
    String(sonar.ping_cm()).toCharArray(ultrasonicData, 255);
    dev.disp().writeMeasurement("co2_sensor", CO2Data);
    dev.disp().writeMeasurement("ultrasonic", ultrasonicData);
    lastMeasurementTime = millis();
  }
}

void gasCalibration() { // Калибровка оптимального уровня углекислого газа
  Serial.println("Calibrating optimal gas levels...");
  fillToWaterSensorLevel();
  CO2Opt = mq135.readCO2();
  delay(10000);
  CO2Opt += mq135.readCO2();
  CO2Opt = CO2Opt / 2;
  Serial.print("Optimal Gas Level(const) = ");
  Serial.println(CO2Opt);
  Serial.println("Finished calibrating.");
  return ;
}
