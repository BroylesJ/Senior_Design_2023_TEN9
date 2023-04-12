#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
#include "user_interface.h"
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

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

float P;
float INHG;
float C;
float F;
int relay;

void setup() {
  Serial.begin(9600);

  // Setup wifi
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

  // Add tags for Influx
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());
  BMP180.addTag("device", "BMP180");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Write Data
  sensor.clearFields();
  sensor.addField("rssi", WiFi.RSSI());

  //This line prints what we are writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  //Checking Wifi Connection
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}


////////////////////START OF MAIN LOOP////////////////////

void loop() {
  BMP180.clearFields();
  getData();
  send_Data();
  printSerial();
  ESP.deepSleep(10e6);
}

void getData(){
  C = bmp.readTemperature(); //read Sensor
  F = (1.8*C) + 32;
  
  P = bmp.readPressure(); //read Sensor
  INHG = P / 3386.4; //conversion factor 
}

void send_Data(){
  BMP180.addField("Temperature", F);
  BMP180.addField("Pressure", INHG);
  client.writePoint(BMP180);
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
