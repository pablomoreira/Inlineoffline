#include <Arduino.h>
//#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
//#include <PubSubClient.h>
#include <TaskScheduler.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include "OtaUtil.hpp"
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <LittleFS.h>
#include "digital.h"
#include "ds18b20.hpp"
#include "config.h"

void cb_wifi();
void cb_ota();
void cb_webserver();
void cb_led();
void cb_searchDs();
void cb_checkTemp();
void cb_prepareHtml();


Signal led1(D5, LOW);


Scheduler runner;

Task task_Wifi(TASK_SECOND * 5, TASK_FOREVER, &cb_wifi, &runner);
Task task_Ota(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_ota, &runner);
Task task_led(TASK_MILLISECOND * 200, TASK_FOREVER, &cb_led, &runner);
Task task_WebServer(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_webserver, &runner);

Task task_checkTemp(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_checkTemp, &runner);
Task task_searchDs(TASK_MILLISECOND * 50, TASK_FOREVER, &cb_searchDs, &runner);

Task taskPrepare(TASK_SECOND, TASK_FOREVER, &cb_prepareHtml, &runner);

ESP8266WebServer server(HTTP_PORT);
void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
void handleReboot();
void handleReset();
void handleSetTemp(String);
String prepareHtml();


Ds18b20 ds(D6);
WiFiManager wm;

void setup() {
  Serial.begin(115200);
  delay(1000);
  bool _reset = false;
  Serial.printf("\n%s\n","--START--");
  bool success = LittleFS.begin();

  if (success) {
    Serial.println("File system mounted with success");
  } else {
    Serial.println("Error mounting the file system");
    return;
  }

  Dir dir = LittleFS.openDir("/");
  //File file = LittleFS.open("/file.txt", "w");
  //LittleFS.remove("/file.txt");
  //file.close();

  while (dir.next()) {
    Serial.println(dir.fileName());
    //str += dir.fileSize();
    //str += "\r\n";
  }
  if (LittleFS.exists(PATH_RESET)) _reset = true;
  LittleFS.end();

  delay(2000);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
      //wm.resetSettings();
  Serial.println(WiFi.macAddress());

  bool res;
      // res = wm.autoConnect(); // auto generated AP name from chipid
      // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  wm.setConfigPortalTimeout(300);
  if(_reset == true )
  {
      Serial.printf("RESET MODE\n");
      wm.resetSettings();
  }

  res = wm.autoConnect(SSID,PASSWRD); // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    LittleFS.begin();
    LittleFS.remove(PATH_RESET);
    LittleFS.end();
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
  server.on(UriBraces("/setTemp/{}"),[](){
    handleSetTemp(server.pathArg(0));
  });
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                      // Actually start the server
  //Serial.println("HTTP server started");
}


void loop() {
  runner.execute();
}

void cb_wifi(){
  //Serial.printf("%d\n", WiFi.status());

  if(WiFi.status() == WL_CONNECTED ){
    if(!task_Ota.isEnabled()){
      task_Ota.enable();
    }
    if(!task_WebServer.isEnabled()){
      task_WebServer.enable();
    }
  }
  else{
    ESP.restart();
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
    if(!taskPrepare.isEnabled()){
      taskPrepare.enable();
    }
  }
}

void handleReboot() {
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  ESP.restart();
}

String prepareHtml(){
  String htmlPage;
  String power = "";
  String ttl;
  uint32_t diff = 0;
  float _temp = 0;
  String temp;

  if (digitalRead(D7) == false){
    power = "ON [POL_ON]";

  }
  else{
    power = "OFF [POL_OFF]";

  }

  _temp = ds.getTemp();
  if (_temp < ds.getTempLimit()){
    temp = String(_temp) + "C [TEMP_OK]";

  }else{
    temp = String(_temp) + "C [TEMP_WARN]";

  }

  diff = millis() - ds.getMark();
  ttl = String(diff);
  if(diff < 1000){
      ttl += " [TTL_OK]";

  }else{
    temp = String(_temp) + "C [TEMP_WARN]";
    ttl += " [TTL_ERROR]";
  }


  htmlPage.reserve(1024);               // prevent ram fragmentation
  htmlPage = "<!DOCTYPE HTML>"
             "<html>"
             "Power Line -> " + power + "<br>"
             "Temp " + String(ds.getTempLimit()) + " -> " + temp + "<br>"
             "TTL -> " + ttl + "<br>"
             "</html>";

  return htmlPage;
}

void handleReset(){
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
  LittleFS.begin();
  LittleFS.open(PATH_RESET, "w");
  LittleFS.remove(PATH_TEMP);
  LittleFS.end();


  ESP.restart();
}
void handleSetTemp(String s_tempLimit){
  if (ds.setTempLimit(s_tempLimit)){
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
  }
  else{
    server.send(404, "text/plain", "404: Out temp range");
    }
}

void cb_prepareHtml(){
  prepareHtml();
}
