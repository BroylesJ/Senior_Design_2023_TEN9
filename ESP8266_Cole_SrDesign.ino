#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
#include "user_interface.h"
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#include <Adafruit_BMP085.h>
#include <Adafruit_INA219.h>

Adafruit_BMP085 bmp;
Adafruit_INA219 ina219_A;
Adafruit_INA219 ina219_B(0x41);

#define seaLevelPressure_hPa 1013.25
#define baud_rate 115200
#define DEVICE "ESP8266"
#define WIFI_SSID "TP-Link_F18D"
#define WIFI_PASSWORD "92835504"
#define INFLUXDB_URL "http://192.168.0.175:8086/"
#define INFLUXDB_TOKEN "8N9AaVkbFPpZNeXjrtMI1y143U-FOQS8CVKUXd0Qjjh34S1TCh_eUvDNV3Hxn8seWRXXmKyAkzurduJMnTaa3w=="
#define INFLUXDB_ORG "10-9"
#define INFLUXDB_BUCKET "Data"
#define TZ_INFO "CST+6CDT,M3.5.0,M10.5.0/3"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

Point sensor("wifi_status");
Point TenNine("Power Harvesters");
Point BMP180("Temperature and Pressure");
Point INA219_A("Solar Current and Voltage");
Point INA219_B("Battery Voltage and Current");

float P;
float INHG;
float C;
float F;
int relay;
float shuntvoltage = 0;
float busvoltage = 0;
float busvoltage2 = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
float batteryCurrent = 0;
float sensorValue = 0;
float vBat = 0;
float SoC = 0;
float ASoC = 50;

void setup() {
  Serial.begin(9600);
  ina219_A.begin();
  ina219_B.begin(); 

   //Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  //Check BMP Connection
  Serial.println();
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP sensor");
    while (1) {
      Serial.print("No BMP Sensor Found");
      Serial.println();
    }
  }
  //check INA29 Connections
  if (! ina219_A.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  if (! ina219_B.begin()) {
    Serial.println("Failed to find INA219_B chip");
    while (1) { delay(10); }
  }
  
  // Add tags for Influx
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());
  BMP180.addTag("device", "BMP180");
  INA219_A.addTag("device", "INA219_A");
  INA219_B.addTag("device", "INA219_B");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Checking Wifi Connection
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

  

////////////////////START OF MAIN LOOP////////////////////

void loop() {
  BMP180.clearFields();
  INA219_A.clearFields();
  INA219_B.clearFields();
  
  getData();
  send_Data();
  printSerial();
  //ESP.deepSleep(10e6);
  delay(1000);
}

void getData(){
  C = bmp.readTemperature(); //read Sensor
  F = (1.8*C) + 32;
  
  P = bmp.readPressure(); //read Sensor
  INHG = P / 3386.4; //conversion factor 

  busvoltage = ina219_A.getBusVoltage_V();
  current_mA = ina219_A.getCurrent_mA();
  power_mW = ina219_A.getPower_mW();
  busvoltage2 = ina219_B.getBusVoltage_V();
  batteryCurrent = ina219_B.getCurrent_mA();
  
  if(busvoltage2 < 1.222){
    SoC = 0.0968 * pow(busvoltage2, 33.145);
  }
  else{
    SoC = (249.79*busvoltage2)-224.06;
  }
}

void send_Data(){
  BMP180.addField("Temperature", F);
  BMP180.addField("Pressure", INHG);
  client.writePoint(BMP180);

  INA219_A.addField("Solar Voltage", busvoltage);
  INA219_A.addField("Solar Current", current_mA);
  INA219_B.addField("Battery Voltage", busvoltage2);
  INA219_B.addField("Battery Current", batteryCurrent);
  INA219_B.addField("State of Charge", SoC);
  client.writePoint(INA219_A);
  client.writePoint(INA219_B);
}

void printSerial(){
  Serial.print("Temperature: ");
  Serial.print(F);
  Serial.print(" degrees F");
  Serial.println();
  Serial.print("Pressure: ");
  Serial.print(INHG);
  Serial.print(" in-Hg");
  Serial.println();
  Serial.print("V Solar = ");
  Serial.print(busvoltage);
  Serial.print(" V");
  Serial.println();
  Serial.print("Current Solar = ");
  Serial.print(current_mA);
  Serial.print(" mA");  
  Serial.println();
  Serial.print("Solar Power = ");
  Serial.print(power_mW);
  Serial.print(" mW");
  Serial.println();
  Serial.print("Battery Voltage = ");
  Serial.print(busvoltage2);
  Serial.print(" V");
  Serial.println();
  Serial.print("SoC = ");
  Serial.print(SoC);
  Serial.println();
  Serial.println();
  }
  
void setupWifi(){
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
}
