#include <SPI.h>      
#include <MFRC522.h>   
#include <ESP8266WiFi.h>
#include <WiFiClient.h>  
#include <ESP8266HTTPClient.h> 
#include <ArduinoJson.h>
#include "DHT.h"

#define RST_PIN  0     
#define SS_PIN  15   
#define DHTPIN 2 
#define DHTTYPE DHT11

MFRC522 mfrc522(SS_PIN, RST_PIN); 
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "#COOR.SISTEMAS-UTS";
const char *password = "26042018";

const char *host = "192.168.0.125";
String serial_number, temperature, humidity;
String node_id = "23TE5290";
byte ReadUID[4]; 

void setup() {
  Serial.begin(115200);    
  SPI.begin();
  dht.begin();    
  mfrc522.PCD_Init();    

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;        
  
  if ( ! mfrc522.PICC_ReadCardSerial()) 
    return;       
    
  Serial.print("UID:");      
  for (byte i = 0; i < mfrc522.uid.size; i++) { 
    if (mfrc522.uid.uidByte[i] < 0x10){  
      Serial.print("0");   
      }
      else{       
      Serial.print("");     
      }
      serial_number.concat(String(mfrc522.uid.uidByte[i], HEX));
      ReadUID[i] = mfrc522.uid.uidByte[i];    
  }
  serial_number.toUpperCase(); 
  Serial.print(serial_number);
  Serial.println("");
  delay(300);
  mfrc522.PICC_HaltA();
    
  HTTPClient http;
  String postData;
  postData = "serial_number=" + serial_number;
  http.begin("http://192.168.0.125:8000/api/cards/auth");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);
  String payload = http.getString();
    
  StaticJsonDocument<200> datos;
  deserializeJson(datos, payload);
  int id = datos["data"]["id"];
  delay(300);

    if (httpCode==200){
        HTTPClient http;
        Serial.println("taking environments parameters");
        String postData2;
        float hum = dht.readHumidity();
        float temp = dht.readTemperature();
        
           if (isnan(hum) || isnan(temp)) {
              Serial.println(F("Failed to read from DHT sensor!"));
              return;
           }
        
        temperature = String(temp);
        humidity = String(hum);
        postData2 = "temperature=" + temperature + "&humidity=" + humidity + "&node_id=" + node_id + "&card_access=" + id;
        http.begin("http://192.168.0.125:8000/api/temps");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int httpCode2 = http.POST(postData2);
        String payload2 = http.getString();
        Serial.println(payload2);
        http.end();
        serial_number = "";
     }
     else{
      String payload3 = http.getString();
      Serial.println(payload3);
      http.end();
      }
  http.end();
  serial_number = "";
  delay(500);  
}
