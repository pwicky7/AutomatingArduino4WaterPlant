#include <Firebase_Arduino_WiFiNINA.h>
 #include "KasaSmartPlug.h"
#define FIREBASE_HOST "acaie-final-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "G46W5RJdv7y9UHDIeZq4WnRp7sCXAUWfqkWnhcgn"
#define WIFI_SSID "iPhone (21)"
#define WIFI_PASSWORD "peepeedog"

FirebaseData firebaseData;

String path1 = "Sensor_full_tank1";
String jsonStr;
int IRreading;
float measurements[5];
float average;
float time_elapsed;

void setup()
{
 pinMode(4, INPUT);
 Serial.begin(9600);
 delay(1000);
 
 Serial.print("Connecting to WiFi…");
 int status = WL_IDLE_STATUS;
 
 while (status != WL_CONNECTED) {
      status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      delay(750); 
      }

 Serial.print(" IP: ");
 Serial.println(WiFi.localIP());
 Serial.println();
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
 Firebase.reconnectWiFi(true);
 
}

void loop()
{
//  
//   float sum = 0;
//   for (int i = 0; i < 5; i++) {
//       measurements[i] = analogRead(A0);
//       sum += analogRead(A0);
//    }
//
//    
//  average = sum/5;
//  Serial.println(average);
//
//    
//time_elapsed = millis()/60000;
jsonStr = "{\"Average IR Measurement\":" + String(average,6) + ", \"Time Elapsed\":" + String(time_elapsed) + "}";
//jsonStr = {String(average)};

float yeet;
yeet = Firebase.get(firebaseData, path1);
Serial.println(yeet);

//if (Firebase.pushJSON(firebaseData, path, jsonStr)) {
if (Firebase.get(firebaseData, path1)) {
 Serial.println(firebaseData.dataPath() + " = " + firebaseData.floatData());
 }
else {
 Serial.println("Error: " + firebaseData.errorReason());
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
 Firebase.reconnectWiFi(true);
 }
 Serial.println();
delay(2000);
 } 
