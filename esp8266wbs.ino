#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <TFT_eSPI.h>
#include <SPI.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <URLCode.h>

#include "allch.h"

#define STASSID "TP-LINK_6E29"
#define STAPSK "dfy6768507"
ESP8266WiFiMulti wifiMulti;
// 建立ESP8266WiFiMulti对象

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite ntptime = TFT_eSprite(&tft);
TFT_eSprite msg = TFT_eSprite(&tft);
uint16_t blue = tft.color565(50, 161, 255);
uint16_t green = tft.color565(47, 232, 139);
uint16_t xred = tft.color565(239, 92, 116);
int request_val = 0;
int lastmillis;
int val = 0;
static const uint8_t LED = D2;

URLCode url;

String turnON = "/page/gpio/1.html";
String turnOFF = "/page/gpio/0.html";

ESP8266WebServer esp8266_server(80);
// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  Serial.begin(9600);
  // 启动串口通讯
  wifiMulti.addAP(STASSID, STAPSK);
  // 将需要连接的一系列WiFi ID和密码输入这里
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  // ESP8266-NodeMCU再启动后会扫描当前网络
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");
  // 环境查找是否有这里列出的WiFi ID。如果有
  Serial.println("Connecting ...");
  // 则尝试使用此处存储的密码进行连接。

  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {  // 尝试进行wifi连接。
    delay(1000);
    Serial.print(i++);
    Serial.print('.');
  }

  // WiFi连接成功后将通过串口监视器输出连接成功信息
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  tft.setCursor(0, 16);
  tft.println("Connected to ");
  tft.setCursor(78, 16);
  tft.setTextColor(blue);
  tft.println(WiFi.SSID());
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 32);
  tft.println("IP:");
  tft.setCursor(20, 32);
  tft.setTextColor(xred);
  tft.println(WiFi.localIP());
  tft.setCursor(0, 48);
  tft.setTextColor(TFT_WHITE);
  tft.println("Get request:");
  tft.setTextColor(green);
  tft.fillRect(73, 46, 45, 9, TFT_BLACK);
  tft.setCursor(73, 48);
  tft.println(request_val);
  ntptime.setColorDepth(8);
  ntptime.createSprite(50, 9);
  ntptime.fillSprite(TFT_BLACK);
  ntptime.setScrollRect(0, 0, 50, 9, TFT_BLACK);
  ntptime.setTextColor(TFT_WHITE);
  // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  // 通过串口监视器输出ESP8266-NodeMCU的IP

  timeClient.begin();

  if (SPIFFS.begin()) {  // 启动闪存文件系统
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
  // 启动闪存文件系统
  // esp8266_server.on("/setLED", handleLED);
  esp8266_server.onNotFound(handleUserRequet);
  // 告知系统如何处理用户请求
    Serial.println("HTTP server started");
    
  esp8266_server.begin();
  // 启动网站服务

}
void loop(void) {
  // 处理用户请求
  lastmillis = millis() + 500;
  timeClient.update();
  ntptime.fillSprite(TFT_BLACK);
  ntptime.drawString(timeClient.getFormattedTime(), 0, 0, 1);
  ntptime.pushSprite(0, 0);
  esp8266_server.handleClient();
  digitalWrite(LED, val);
}
void handleUserRequet() {
  // 获取用户请求网址信息
  String webAddress = esp8266_server.uri();
  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);
  request_val += 1;
  tft.setTextColor(green);
  tft.fillRect(73, 46, 45, 9, TFT_BLACK);
  tft.setCursor(73, 48);
  tft.println(request_val);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK) {
    esp8266_server.send(404, "text/plain", "404 Not Found");
  }
}
//处理浏览器HTTP访问
bool handleFileRead(String path) {
  Serial.println(path);
  // 如果访问地址以"/"为结尾
  if (path.endsWith("/")) {
    path = "/index.html";
    // 则将访问地址修改为/index.html便于SPIFFS访问
 } else if (path == turnON) {
   Serial.println("get it!");
   val = 1;
 } else if (path == turnOFF) {
   Serial.println("get it!");
   val = 0;
 }
    else if (path.endsWith("&")) {

    int sy = path.indexOf("&");
    path.setCharAt(0, ' ');
    path.setCharAt(1, ' ');
    path.setCharAt(2, ' ');
    path.setCharAt(3, ' ');
    path.setCharAt(4, ' ');
    path.setCharAt(5, ' ');
    path.setCharAt(sy, ' ');
    path.replace(" ", "");
    url.urlcode = path;
    // Decoding the urlcode
    url.urldecode();
    String st = url.strcode;
    // Gain the decoded URL
    Serial.print("it is that!\n");
    Serial.println(st);
    tft.fillScreen(TFT_BLACK);
    tft.loadFont(font_12);
    tft.setCursor(0, 75);
    tft.println(st);
    tft.unloadFont();
    tft.setCursor(0, 16);
    tft.println("Connected to ");
    tft.setCursor(78, 16);
    tft.setTextColor(blue);
    tft.println(WiFi.SSID());
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 32);
    tft.println("IP:");
    tft.setCursor(20, 32);
    tft.setTextColor(xred);
    tft.println(WiFi.localIP());
    tft.setCursor(0, 48);
    tft.setTextColor(TFT_WHITE);
    tft.println("Get request:");
    tft.setTextColor(green);
    tft.fillRect(73, 46, 45, 9, TFT_BLACK);
    tft.setCursor(73, 48);
    tft.println(request_val);
    ntptime.setColorDepth(8);
    ntptime.createSprite(50, 9);
    ntptime.fillSprite(TFT_BLACK);
    ntptime.setScrollRect(0, 0, 50, 9, TFT_BLACK);
    ntptime.setTextColor(TFT_WHITE);

    path = "/page/ArduinoZone.html";
  }

  String contentType = getContentType(path);
  // String re = "replace";
  // 获取文件类型
  if (SPIFFS.exists(path)) {
    // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(path, "r");
    // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);
    // 并且将该文件返回给浏览器
    file.close();
    // 并且关闭文件

    return true;
    // 返回true
  }
  // else if(re == contentType)
  // {
  //   return true;
  // }
  return false;
  // 如果文件未找到，则返回false
}
// 获取文件类型
String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  // else if(filename.endsWith("&")) return "replace";
  return "text/plain";
}
