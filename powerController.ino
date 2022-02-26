#include <Wire.h> //Bibliothek für die kommunikation mit der RTC
#define RTC_I2C_ADDRESS 0x68 // I2C Adresse des RTC  DS3231

#define SWITCHRELAY 3
#define POWERRELAY 4
#define BATTERY A7
#define POWERSENSOR A3
#define FAN 5

#define VoltageAverageSize 100

float voltage[VoltageAverageSize];
int voltageIndex = 0;
float voltageAvg = 0;
long voltageTimeOld;

bool on = true;
bool powered = false;

long brownOutOld;

long lastReadRTC;

void setup() {
  Serial.begin(19200);

  Wire.begin(); 
  
  pinMode(SWITCHRELAY, OUTPUT);
  pinMode(POWERRELAY, OUTPUT);
  pinMode(FAN, OUTPUT);
  
  pinMode(BATTERY, INPUT);
  pinMode(POWERSENSOR, INPUT);
  
  digitalWrite(SWITCHRELAY, LOW);
  digitalWrite(POWERRELAY, LOW);
  digitalWrite(FAN, LOW);
}

void loop() {
  if((lastReadRTC + 5000) < millis()){
    lastReadRTC = millis();
    on = checkTime();
  }
  if(on){
    if(powered){
      digitalWrite(SWITCHRELAY, HIGH);
      digitalWrite(POWERRELAY, LOW);
      digitalWrite(FAN, LOW);
    }
    else{
      digitalWrite(SWITCHRELAY, LOW);
      if(millis() >= brownOutOld + 100){
        brownOutOld = millis();
        Serial.println(voltageAvg);
        if(voltageAvg <= 10.9){
          digitalWrite(POWERRELAY, LOW);
          digitalWrite(FAN, LOW);
        }
        else if(voltageAvg >= 11.5){
          digitalWrite(POWERRELAY, HIGH);
          analogWrite(FAN, 200);
        }
      }
    }
    if(millis() >= voltageTimeOld + 10){
      voltageTimeOld = millis();
      batteryVoltage();
    } 
    powered = !(analogRead(POWERSENSOR) >= 500);
  }
  else{
    digitalWrite(SWITCHRELAY, LOW);
    digitalWrite(POWERRELAY, LOW);
    digitalWrite(FAN, LOW);
  }
}

bool checkTime(){
  bool weekend = false;
  Wire.beginTransmission(RTC_I2C_ADDRESS); //Aufbau der Verbindung zur Adresse 0x68
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
 int sekunde    = bcdToDec(Wire.read() & 0x7f);
 int minute     = bcdToDec(Wire.read()); 
 int stunde     = bcdToDec(Wire.read() & 0x3f); 
  //Der Wochentag wird hier nicht ausgelesen da dieses mit 
  //dem Modul RTC DS3231 nicht über die Wire.h zuverlässig funktioniert.
 int wochentag  = bcdToDec(Wire.read());
 int tag        = bcdToDec(Wire.read());
 int monat      = bcdToDec(Wire.read());
 int jahr       = bcdToDec(Wire.read())+2000;  
 
 if(wochentag == 0 || wochentag == 6){
   weekend = true;
 }
 return weekend;
}

void batteryVoltage(){
  if(voltageIndex > VoltageAverageSize - 1){
    voltageIndex = 0;
  }
  float v = (analogRead(BATTERY) * 5.1) / 1024.0;
  v = v/30 *98;
  voltage[voltageIndex] = v;
  voltageIndex ++;
  
  voltageAvg = 0;
  for(int i = 0; i<= VoltageAverageSize - 1; i++){
    voltageAvg += voltage[i];
  }
  voltageAvg = voltageAvg / VoltageAverageSize;
}
byte bcdToDec(byte val){
  return ( (val/16*10) + (val%16) );
}
