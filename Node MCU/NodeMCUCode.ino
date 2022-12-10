#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//WiFi Configuration
#define WIFI_SSID "Subsidi"
#define WIFI_PASSWORD "88888888"

//Firebase Configuration
#define API_KEY "AIzaSyCl0oQYGEespQiaLH27oRTVIc58Rxln03s"
#define DATABASE_URL "https://smart-garden-tekkom-default-rtdb.asia-southeast1.firebasedatabase.app/"

int wifi_connection = D2;
int firbase_connection = D3; 
int sanding_indicator = D4;

int input = 0;
char lastTriger[2];
char inputChar;
char type[] = "THSP|";
String tmp; 
float tempe, huma, soil;
int pumpStat = LOW;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signUpOk = false;


float alpha(float T, float RH){
  return log(RH / 100) + ((17.625 * T)/(243.04 + T));
}

float dewPoint(float T, float RH){
  return (243.04 * alpha(T,RH)) / (17.625 - alpha(T,RH));
}

float c[] = {-8.78469475556, 1.61139411, 2.33854883889, -0.14611605, -0.012308094, -0.0164248277778, 0.002211732,0.00072546,-0.000003582};

float heatIndex(float T, float RH){
    return float(c[0]) + (float(c[1]) * T) + (float(c[2]) * RH) + (float(c[3]) * T * RH) + (float(c[4]) * pow(T,2)) + (float(c[5]) * pow(RH,2)) + (float(c[6]) * pow(T,2) * RH) + (float(c[7]) * T * pow(RH,2)) + (float(c[8]) * pow(T,2) * pow(RH,2));
}

void setup() {
  Serial.begin(9600);
  
  pinMode(wifi_connection, OUTPUT);
  pinMode(firbase_connection, OUTPUT);
  pinMode(sanding_indicator, OUTPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){ 
    delay(100);
    digitalWrite(wifi_connection, LOW);
    Serial.print("-");
  }
  Serial.println("-> Connected!");
  digitalWrite(wifi_connection, HIGH);
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    signUpOk = true;
    Serial.println("Firebase Oke!");
    digitalWrite(firbase_connection, HIGH);
  }else{
    digitalWrite(firbase_connection, LOW);
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  FirebaseJson json;
  while (Serial.available() > 0){
    input = Serial.read(), DEC;
    inputChar = char(input);
    if(check()){
      lastTriger[1] = lastTriger[0];
      lastTriger[0] = inputChar;      
    }
    for(int i=0; i < 5; i++){
      add(type[i]);
    }
    if(lastTriger[0] == '|'){
      switch (lastTriger[1]) {
        case 'T':  // Suhu
          tempe = tmp.toFloat(); 
          break;
        case 'H':  // Kelembapan Udara
          huma = tmp.toFloat();
          break;
        case 'S':  // Kelembapan Tanah 
          soil = tmp.toFloat();
          break;
        case 'P':  // Kelembapan Tanah 
          pumpStat = tmp.toInt();
          break;
      }
      tmp="";
    }
  }
    if(Firebase.ready() && signUpOk){
      if(tempe && soil && huma){
        Serial.println("Dapat!");
        json.add("temperature",tempe);
        json.add("air_humadity",huma);
        json.add("soil_humadity",soil);
        json.add("daw_point",dewPoint(tempe, huma));
        json.add("head_index",heatIndex(tempe, huma));
        Firebase.RTDB.setBoolAsync(&fbdo,"/pump_status", pumpStat);
        if(Firebase.RTDB.pushJSON(&fbdo,"/sensor", &json)){
          String n = fbdo.pushName();
          if(Firebase.RTDB.pushTimestamp(&fbdo, ("/sensor/"+n))){
            if(Firebase.RTDB.getString(&fbdo,"/sensor/"+n+"/"+fbdo.pushName())){
              Firebase.RTDB.setIntAsync(&fbdo,"/sensor/"+n+"/timestamp", fbdo.to<int>());
            }
          }
          digitalWrite(sanding_indicator, HIGH);
          Serial.println("Terkirim");
          delay(75);
        } 
        digitalWrite(sanding_indicator, LOW);
      }
    }
}

int check(){
  for(int i=0; i < 5; i++){
     if(inputChar == type[i]){
      return 1;
     }
  }
  return 0;
}

void add(char y){
     if((lastTriger[0] == y) && !check()){
      tmp += String(inputChar); 
    }
}
