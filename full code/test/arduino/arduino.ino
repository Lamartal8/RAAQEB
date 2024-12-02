#include <EEPROM.h>
//gsm
#include <SoftwareSerial.h>
SoftwareSerial sim(4, 5);
String number_call = "+966530103670";
String number_sms1 = "+966502915792";
String number_sms2 = "+966539010400";
String number_sms3 = "+966561895785";
String number_sms4 = "+966531688656";

//gps
#include "TinyGPS++.h"
SoftwareSerial sgps(3, 2);

TinyGPSPlus gps;
String Latitude = "", Longitude = "";
String saved_Latitude = "", saved_Longitude = "";

#define fire1_pin 10
#define fire2_pin 11
#define gas1_pin A0
#define gas2_pin A1
#define buzz 13
int fire1, fire2, gas1, gas2;

unsigned long read_sensor_delay = 0, send_data_delay = 0, send_msg_delay = 0, call_delay = 0;

SoftwareSerial wifi(6, 7);
int alarm_status = 0;


#include "DHT.h"
#define DHTPIN1 8
#define DHTPIN2 9
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

int temp1, hum1;
int temp2, hum2;

void setup() {
  Serial.begin(9600);
  wifi.begin(115200);
  sim.begin(9600);
  sgps.begin(9600);
  sgps.listen();
  dht1.begin();
  dht2.begin();

  pinMode(fire1, INPUT);
  pinMode(fire2, INPUT);
  pinMode(gas1, INPUT);
  pinMode(gas2, INPUT);
  pinMode(buzz, OUTPUT);
}


void loop() {

  get_gps();
  read_sensor();


}


void read_sensor() {

  if (millis() - read_sensor_delay > 1000) {

    fire1 = digitalRead(fire1_pin);
    fire2 = digitalRead(fire2_pin);
    gas1 = analogRead(gas1_pin);
    gas2 = analogRead(gas2_pin);

    gas1=map(gas1,300,1023,0,100); 
    gas1=constrain(gas1,0,100);
    gas2=map(gas2,300,1023,0,100);
    gas2=constrain(gas2,0,100);

    int test_temp = dht1.readTemperature();
    int test_hum = dht1.readHumidity();
    if (isnan(test_temp) || isnan(test_hum)) {
      return;
    }
    temp1 = test_temp;
    hum1 = test_hum;

    test_temp = dht2.readTemperature();
    test_hum = dht2.readHumidity();
    if (isnan(test_temp) || isnan(test_hum)) {
      return;
    }
    temp2 = test_temp;
    hum2 = test_hum;


    Serial.print("F1: ");
    Serial.print(fire1);
    Serial.print("  F2: ");
    Serial.print(fire2);
    Serial.print("  G1: ");
    Serial.print(gas1);
    Serial.print("  G2: ");
    Serial.print(gas2);
    Serial.print("  T1: ");
    Serial.print(temp1);
    Serial.print(" H1: ");
    Serial.print(hum1);
    Serial.print("  T2: ");
    Serial.print(temp2);
    Serial.print(" H2: ");
    Serial.print(hum2);

    Serial.println();

    //////////////////

    read_sensor_delay = millis();
  }
}


void get_gps() {
  sgps.listen();
  while (sgps.available()) {
    gps.encode(sgps.read());
  }
  if (gps.location.isUpdated()) {

    Latitude = String(gps.location.lat(), 6);
    Longitude = String(gps.location.lng(), 6);

    /*  Serial.print("Latitude: ");
    Serial.print(Latitude);
    Serial.print("  Longitude: ");
    Serial.println(Longitude);*/

    //write to eeprom
    write_to_memory_lat(Latitude);
    write_to_memory_lon(Longitude);


  } else {
    //read from eeprom
    saved_Latitude = read_from_memory_lat();
    saved_Longitude = read_from_memory_lon();
    Serial.print("saved_Latitude: ");
    Serial.print(saved_Latitude);
    Serial.print("  saved_Longitude: ");
    Serial.println(saved_Longitude);
  }
}


String read_from_memory_lat() {
  // 35.132456
  String num = "";
  num += (char)EEPROM.read(1);
  num += (char)EEPROM.read(2);
  num += '.';
  num += (char)EEPROM.read(3);
  num += (char)EEPROM.read(4);
  num += (char)EEPROM.read(5);
  num += (char)EEPROM.read(6);
  num += (char)EEPROM.read(7);
  num += (char)EEPROM.read(8);
  return num;
}

String read_from_memory_lon() {

  String num = "";
  num += (char)EEPROM.read(11);
  num += (char)EEPROM.read(12);
  num += '.';
  num += (char)EEPROM.read(13);
  num += (char)EEPROM.read(14);
  num += (char)EEPROM.read(15);
  num += (char)EEPROM.read(16);
  num += (char)EEPROM.read(17);
  num += (char)EEPROM.read(18);
  return num;
}

void write_to_memory_lat(String num) {
  //xx,xxxxxx
  EEPROM.write(1, num[0]);
  EEPROM.write(2, num[1]);

  EEPROM.write(3, num[3]);
  EEPROM.write(4, num[4]);
  EEPROM.write(5, num[5]);
  EEPROM.write(6, num[6]);
  EEPROM.write(7, num[7]);
  EEPROM.write(8, num[8]);
}

void write_to_memory_lon(String num) {
  //xx,xxxxxx
  EEPROM.write(11, num[0]);
  EEPROM.write(12, num[1]);

  EEPROM.write(13, num[3]);
  EEPROM.write(14, num[4]);
  EEPROM.write(15, num[5]);
  EEPROM.write(16, num[6]);
  EEPROM.write(17, num[7]);
  EEPROM.write(18, num[8]);
}

void send_sms(String number) {
  Serial.println(">>>>>>>>>>>>>> send sms >>>>>>>>>>>>>>>>");
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + number + "\"\r");
  delay(100);
  sim.println("There is an emergency level " + String(alarm_status) + ", please evacuate the building immediately.");
  sim.println((char)26);
  delay(3000);
}

void call_number(String number) {
  sim.print(F("ATD"));
  sim.print(number);
  sim.print(F(";\r\n"));
  //delay(10000);
  //sim.println("ATH");
}
