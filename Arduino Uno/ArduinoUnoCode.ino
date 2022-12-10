#include <HTU21D.h>
#include <SoftwareSerial.h>
HTU21D sensor;
SoftwareSerial espSerial(5, 6);
void setup() {
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  sensor.begin();
  espSerial.begin(9600);
}

int input =0 ;
char lastTriger;
char tampil; 
String str;
int pumpStatus = LOW;

void loop() {
  if(sensor.measure()) {
    float tempe = sensor.getTemperature();
    float humi = sensor.getHumidity();
    float humi2 = analogRead(A1);
    Serial.print("Temperature (°C): ");
    Serial.println(tempe);    
    Serial.print("Humidity (%RH): ");
    Serial.println(humi);
    Serial.print("Soil Humidity 2 (%RH): ");
    humi2 = calcSoilHum(humi2);
    Serial.println(humi2);
    str = String('T')+String(tempe)+String("|H")+String(humi)+String("|S")+String(humi2)+String("|P")+String(pumpStatus)+String("|");
    espSerial.println(str);
    if(((tempe < 30) && (humi > 85)) || (humi2 > 80)){
      pumpStatus = LOW;
    }
    
    if(((tempe > 35) && (humi < 85)) || (humi2 <= 80)){
      pumpStatus = HIGH;
    }
    digitalWrite(4, (!pumpStatus));
  }
  delay(1000);
}

float calcSoilHum(float sensorValue){
  float result = sensorValue;
  if(result < 290){
     return 100;
  }
  result = sensorValue-290;
  result = (result/733)*100;
  result = 100 - result; 
  return result;
}
