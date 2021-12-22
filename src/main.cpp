#include <Arduino.h>
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include <TaskScheduler.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include "OtaUtil.hpp"
#include <ESP8266WebServer.h>
#include "digital.h"
#include "ds18b20.hpp"
#include "config.h"

void cb_wifi();
void cb_ota();
void cb_webserver();
void cb_led();
void cb_searchDs();
void cb_checkTemp();

Signal led1(D5, LOW);


Scheduler runner;

Task task_Wifi(TASK_SECOND * 5, TASK_FOREVER, &cb_wifi, &runner);
Task task_Ota(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_ota, &runner);
Task task_led(TASK_MILLISECOND * 200, TASK_FOREVER, &cb_led, &runner);
Task task_WebServer(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_webserver, &runner);

Task task_checkTemp(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_checkTemp, &runner);
Task task_searchDs(TASK_MILLISECOND * 50, TASK_FOREVER, &cb_searchDs, &runner);


ESP8266WebServer server(HTTP_PORT);
void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

Ds18b20 ds(D6);

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  WiFiManager wm;
      //wm.resetSettings();


  bool res;
      // res = wm.autoConnect(); // auto generated AP name from chipid
      // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect(SSID,PASSWRD); // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
  task_Wifi.enable();
  task_led.enable();
  task_searchDs.enable();
  led1.setblink(1);

  InitOTA();
  pinMode(D7,INPUT);

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  //Serial.println("HTTP server started");
}


void loop() {
  runner.execute();
}

void cb_wifi(){
  if(!task_Ota.isEnabled()){
    task_Ota.enable();
  }
  if(!task_WebServer.isEnabled()){
    task_WebServer.enable();
  }
}

void cb_ota(){
  ArduinoOTA.handle();
}

void handleRoot() {
  bool power = digitalRead(D7);
  String msg = "";
  String info = "\nIndex Sensor -> " + String(ds.getNum()) + " \n Address Sensor -> " + String(ds.addr2str()) + " " + String(ds.getTemp());
  info = info + "\n" + String(millis() - ds._mark_time);
  if (power == false){
    msg = "Power On Line";
  }
  else{
    msg = "Power Off Line";
  }
  msg += info;
  server.send(200, "text/plain", msg);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void cb_webserver(){
  server.handleClient();
}

void cb_led(){
    led1.blink();
}

void cb_searchDs() {
  ds.search();
  uint8_t n = ds.getNum();
  if(n > 0 && ds.crc8()){
    task_searchDs.disable();
    task_checkTemp.enable();
  }
  else{
    task_checkTemp.disable();
    task_searchDs.enable();
  }
}

void cb_checkTemp() {
  if (ds.update()){
    task_checkTemp.disable();
    task_searchDs.enable();
  }
}
