#include <SPI.h>      
#include <MFRC522.h>   
#include <ESP8266WiFi.h>
#include <WiFiClient.h>  
#include <ESP8266HTTPClient.h> 

#define RST_PIN  5     
#define SS_PIN  15      
MFRC522 mfrc522(SS_PIN, RST_PIN); 

const char *ssid = "WIMACO";
const char *password = "9148554800";
const char *host = "192.168.1.53";
String serial_number, temperature;
String node_id = "23TE5290";
byte ReadUID[4]; 
void setup() {
  Serial.begin(115200);    
  SPI.begin();     
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
    http.begin("http://192.168.1.53:8000/api/cards/" + serial_number);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.GET();
    String payload = http.getString();

    Serial.println(payload);
    if (httpCode == 200){
       HTTPClient http;
       Serial.println("taking temperature");
           String postData;
           float temp = analogRead(A0);
           temp = (3.3 * temp * 100.0)/1024.0;
           temperature = String(temp);
           postData = "temperature=" + temperature + "&node_id=" + node_id + "&card_access=" + serial_number;
           http.begin("http://192.168.1.53:8000/api/temps");
           http.addHeader("Content-Type", "application/x-www-form-urlencoded");
           int httpCode = http.POST(postData);
           String payload = http.getString();
           Serial.println(httpCode);
           Serial.println(payload);
           http.end();
           serial_number = "";
           return;
      }
      
    http.end();
    serial_number = "";
    delay(500);   
}
