#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//#include <stdio.h>
const char* ssid = "kolesnik"; //Подключается к точке доступа OpenWrt
const char* password = "ots.kh.ua";
char buf[30]; 
long sec;
int ss;
String addr_ds="";
int numberOfDevices;


//IPAddress ip(192,168,88,23);
//IPAddress gateway(192,168,88,1);
//IPAddress subnet(255,255,255,0); 
ESP8266WebServer server(8080);
OneWire ds(0); // Датчик температуры DS18B20 на GPIO - 0
#define ONE_WIRE_BUS 0
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress  DS18B20;
char out_temp[1000];

const int led1 = 1; // GPIO - 1 LED R
const int led2 = 2; // GPIO - 2 LED W

void temper() { 
  sensors.requestTemperatures();
  sprintf(out_temp, "","");
  for ( int ii=0; ii<numberOfDevices; ii++) {
      //String adr_cur=printAddress(123);
      String adr_cur="";
      if (!sensors.getAddress(DS18B20, ii)) 
      {
        adr_cur="adr_didnt_read";
      }else {
        for (uint8_t i = 0,j=0; i < 8; i++)
        {
          if (DS18B20[ii] < 16) adr_cur+="0";
          adr_cur+=String(DS18B20[i],HEX);
        }
      }
      float celsius=sensors.getTempCByIndex(ii);
      char str_temp[6];
      dtostrf(celsius, 4, 2, str_temp);
      sprintf(out_temp, "%s\"%s\":\"%s\",",out_temp,adr_cur.c_str(),str_temp);
  }
}

 
void tem(){
  char send_temp[1000];
  sprintf(send_temp, "{%s}", out_temp); 
  server.send(200, "text/x-json",send_temp);
}
void whi_on() {
  digitalWrite(led2, 1);
  server.send(200, "text/x-json","{\"gpio2\":\"1\"}");
}
void whi_off() {
  digitalWrite(led2, 0);
  //server.send(200, "text/html","<h1>whi of</h1>");
  server.send(200, "text/html","{\"gpio2\":\"0\"}");
}
void red_on() {
  digitalWrite(led1, 0);
  //server.send(200, "text/html","<h1>red on</h1>");
  server.send(200, "text/x-json","{\"gpio1\":\"1\"}");
}
void red_off() {
  digitalWrite(led1, 1);
  ///server.send(200, "text/html","<h1>red of</h1>");
  server.send(200, "text/x-json","{\"gpio1\":\"0\"}");
}
void whi_sta(){
  if (digitalRead(led1)==0 && digitalRead(led2)==1) server.send(200, "text/x-json","{\"gpio1\":\"1\",\"gpio2\":\"1\"}");
  if (digitalRead(led1)==1 && digitalRead(led2)==1) server.send(200, "text/x-json","{\"gpio1\":\"0\",\"gpio2\":\"1\"}");
  if (digitalRead(led1)==0 && digitalRead(led2)==0) server.send(200, "text/x-json","{\"gpio1\":\"1\",\"gpio2\":\"0\"}");
  if (digitalRead(led1)==1 && digitalRead(led2)==0) server.send(200, "text/x-json","{\"gpio1\":\"0\",\"gpio2\":\"0\"}");
}
void root(){
  server.send(200, "text/html","<h1>This is web-server on Wifi-esp-01.<br>He controls two led and temperature.<br>Command:/io1on /io1off /io2on /io2off /temp /status</h1>"); 
}
void setup(void){
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
//  Serial.print("Found ");
//  Serial.print(numberOfDevices, DEC);
//  Serial.println(" devices.");
  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, 1);//1
  digitalWrite(led2, 0);//0
  //WiFi.config(ip,gateway,subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname("esp-wifi-relay");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  temper();
  server.on("/temp", tem);
  server.on("/io2on", whi_on);
  server.on("/io2off", whi_off);
  server.on("/io1on", red_on);
  server.on("/io1off", red_off);
  server.on("/status", whi_sta);
  server.on("/", root); 
  server.begin();
  // Serial.println("HTTP server started");
}
void loop(void){ 
 sec=millis()/1000;
 ss=sec%60; // second
 if(ss==0 || ss==30 ) {
  temper(); 
 }
 server.handleClient(); 
}
