
#include <Firebase_Arduino_WiFiNINA.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Ezo_i2c.h>                    //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
#include <Wire.h>


#define FIREBASE_HOST "acaie-final-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "G46W5RJdv7y9UHDIeZq4WnRp7sCXAUWfqkWnhcgn"

//////////////////////////
// Important 
// Fill in Wifi 
#define WIFI_SSID "iPhone (21)"
#define WIFI_PASSWORD "peepeedog"

// Triple Sensor Calibration
float ec_max = 10000;
float ph_max= 10;
float do_max= 10000;
float ec_min = 0;
float ph_min= 2;
float do_min= 0;


// Firebase path names
String path3 = "Manual Stop";
String path1 = "Tank Status";
String path2 = "MF Status";
String pathec = "EC Status";
String pathecnum = "EC Data";
String pathph = "pH Status";
String pathphnum = "pH Data";
String pathdo = "DO Status";
String pathdonum = "DO Data";



LiquidCrystal_I2C lcd(0x27, 16, 2);

FirebaseData firebaseData;
int solenoidPin = 3;
float measurements1[5];
float measurements2[5];
float measurements3[5];
float measurements4[5];
float measurements5[5];
float measurements6[5];
float average1;
float average2;
float average3;
float average4;
float average5;
float average6;
float ph_data;
float ec_data;
float do_data;
float time_elapsed;
String jsonStr;
int IRreading;
bool manual_stop;
bool pump_on = true;

Ezo_board ph = Ezo_board(99, "PH");     //create a PH circuit object, who's address is 99 and name is "PH"
Ezo_board DO = Ezo_board(97, "DO"); //create an RTD Temperature circuit object who's address is 102 and name is "TEMP"
Ezo_board ec = Ezo_board(100, "EC"); //add as many EZO devices as you have

void setup()
{
  Wire.begin(); 
  Serial.begin(9600);
  lcd.begin();
  pinMode(solenoidPin, OUTPUT);

  // Connect to Wifi and Firebase
  Serial.print("Connecting to WiFi…");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(750); 
  }
  status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(" IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

  //Set Firebase names
  Firebase.setInt(firebaseData, path3, 1);
}

void loop()
{  
  manual_stop_check();

  read_level_sensors();

  read_triple_sensors();

  //test results of sensors
  lcd.begin();
  if (average1 > 1020){
    lcd.print("Tank Full, ");
    Firebase.setString(firebaseData, path1, "Tank Full");
    pump_on = false;
  }
  else {
    Firebase.setString(firebaseData, path1, "Tank Acceptable");
  }
  if (average5 > average6){
    lcd.print("Replace MF, ");
    Firebase.setString(firebaseData, path2, "Replace MF");
    pump_on = false;
  }
  else {
    Firebase.setString(firebaseData, path2, "Filter Acceptable");
  }
  // triple sensor data
  if (ec_data < ec_min || ec_data > ec_max){
    lcd.print("Fix EC, ");
    Firebase.setString(firebaseData, pathec, "Fix EC");
    pump_on = false;
  }
  else {
    Firebase.setString(firebaseData, pathec, "EC Acceptable");
  }
  if (ph_min > ph_data || ph_data > ph_max){
    lcd.print("Fix pH, ");
    Firebase.setString(firebaseData, pathph, "Fix pH");
    pump_on = false;
  }
  else {
    Firebase.setString(firebaseData, pathph, "pH Acceptable");
  }
  if (do_data < do_min || do_data > do_max){
    lcd.print("Fix DO, ");
    Firebase.setString(firebaseData, pathdo, "Fix DO");
    pump_on = false;
  }
  else {
    Firebase.setString(firebaseData, pathdo, "DO Acceptable");
  }

  pump_action();
  pump_on = true;
}

void pump_action(){
  if (manual_stop){
    //close valve
    digitalWrite(solenoidPin, HIGH);
    Firebase.setString(firebaseData, "Pump", "Manually Paused");
    lcd.print("Manually Paused"); 
    Serial.println("Manually Paused");

  }
  else if (pump_on){
    //open valve
    digitalWrite(solenoidPin, LOW);
    Serial.println("Pump On");
    lcd.print("Pump Operating");
    Firebase.setString(firebaseData, "Pump", "Pump Operating"); 

  }
  else{
    //close valve
    digitalWrite(solenoidPin, HIGH); 
//    Serial.println("Pump Off");
    lcd.print("Pump Paused");
    Firebase.setString(firebaseData, "Pump", "Pump Paused"); 

  }

}

bool manual_stop_check(){
  // get variables from firebase
//  Serial.println(Firebase.get(firebaseData,path3));
//  Serial.println(firebaseData.dataPath() + " = " + firebaseData.intData());
  if (firebaseData.intData() == 0) {
    manual_stop = false;
  }
  else{
    manual_stop = true;
  }
}

void read_level_sensors(){
  float sum1 = 0;
  float sum2 = 0;
  //float sum3 = 0;
  //float sum4 = 0;
  float sum5 = 0;
  float sum6 = 0;
  for (int i = 0; i < 5; i++) {
    measurements1[i] = analogRead(A0);
    measurements2[i] = analogRead(A1);
    //measurements3[i] = analogRead(A2);
    //measurements4[i] = analogRead(A3);
    measurements5[i] = analogRead(A4);
    measurements6[i] = analogRead(A5);
    sum1 += analogRead(A0);
    sum2 += analogRead(A1);
    //sum3 += analogRead(A2);
    //sum4 += analogRead(A3);
    sum5 += analogRead(A0);
    sum6 += analogRead(A1);
    average1 = sum1/5;
    average2 = sum2/5;
    //average3 = sum3/5;
    //average4 = sum4/5;
    average5 = sum5/5;
    average6 = sum6/5 + 1000;
    }
//  Serial.println("read level sensors");
}

void read_triple_sensors(){
  ph.send_read_cmd();
  ec.send_read_cmd();
  DO.send_read_cmd();

  // read command needs a second to process
  delay(1000);                          

  //get the reading from the PH circuit
  ph_data = receive_reading(ph);
  Firebase.setFloat(firebaseData, pathphnum, ph_data);
  Serial.print("  ");

  //get the reading from the DO circuit
  do_data = receive_reading(DO);
  Firebase.setFloat(firebaseData, pathdonum, do_data);
  Serial.print("  ");
  
  //get the reading from the EC circuit
  ec_data = receive_reading(ec);
  Firebase.setFloat(firebaseData, pathecnum, ec_data);
  Serial.println();

}

// function to decode the reading after the read command was issued
float receive_reading(Ezo_board &Sensor) {

  Serial.print(Sensor.get_name()); Serial.print(": ");  // print the name of the circuit getting the reading
  Sensor.receive_read_cmd();                            // get the response data

  switch (Sensor.get_error()) {                         // switch case based on what the response code is.
    case Ezo_board::SUCCESS:
      Serial.print(Sensor.get_last_received_reading()); //the command was successful, print the reading
      return Sensor.get_last_received_reading();
      break;

    case Ezo_board::FAIL:
      Serial.print("Failed ");                          //means the command has failed.
      return -1;
      break;

    case Ezo_board::NOT_READY:
      Serial.print("Pending ");                         //the command has not yet been finished calculating.
      return -2;
      break;

    case Ezo_board::NO_DATA:
      Serial.print("No Data ");                         //the sensor has no data to send.
      return -3;
      break;
  }
}

// based on an example by Atlas Scientific
// https://github.com/Atlas-Scientific/Ezo_I2c_lib/blob/master/Examples/I2c_read_mulitple_circuits/I2c_read_mulitple_circuits.ino
