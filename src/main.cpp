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
void handleReboot();
void handleReset();
String prepareHtml();
Ds18b20 ds(D6);
WiFiManager wm;

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
      //wm.resetSettings();


  bool res;
      // res = wm.autoConnect(); // auto generated AP name from chipid
      // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  wm.setConfigPortalTimeout(300);
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
  server.on("/reboot",handleReboot);
  server.on("/reset",handleReset);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                      // Actually start the server
  //Serial.println("HTTP server started");
}


void loop() {
  runner.execute();
}

void cb_wifi(){
  if(WiFi.status() != WL_CONNECTION_LOST ){
    if(!task_Ota.isEnabled()){
      task_Ota.enable();
    }
    if(!task_WebServer.isEnabled()){
      task_WebServer.enable();
    }
    else{;}
  }
}

void cb_ota(){
  ArduinoOTA.handle();
}

void handleRoot() {

  //String msg = "";
  //String info = "\nIndex Sensor -> " + String(ds.getNum()) + " \nAddress Sensor -> " + String(ds.addr2str()) + " " + String(ds.getTemp());
  //info = info + "\n" + String(millis() - ds._mark_time);
  //msg += info;
  server.send(200, "text/html", prepareHtml());   // Send HTTP status 200 (Ok) and send some text to the browser/client

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

void handleReboot() {
  ESP.restart();
}

String prepareHtml(){
  String htmlPage;
  String power = "";
  String ttl;
  uint32_t diff = 0;

  if (digitalRead(D7) == false){
    power = "POL_UP";
  }
  else{
    power = "POL_DOWN";
  }
  diff = millis() - ds.getMark();
  ttl = String(diff);
  if(diff < 1000){
      ttl += " TTL_OK";
  }else{
    ttl += " TTL_ERROR";
  }

  htmlPage.reserve(1024);               // prevent ram fragmentation
  htmlPage = "<!DOCTYPE HTML>"
             "<html>"
             "Power On Line -> " + power + "<br>";

  htmlPage +="Temperature -> " + String(ds.getTemp()) + "<br>";
  htmlPage +="TTL -> " + ttl + "<br>";
  htmlPage +="Count -> " + String(ds.getNum());


             htmlPage += "</html>";
    return htmlPage;
}
void handleReset(){
  //wm.resetSettings();
  ESP.restart();
}
