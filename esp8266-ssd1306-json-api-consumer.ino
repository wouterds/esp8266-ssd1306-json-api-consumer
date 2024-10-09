#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "env.h"

const int DISPLAY_WIDTH = 128;
const int DISPLAY_HEIGHT = 64;
const int DISPLAY_ADDRESS = 0x3C;
const char* NTP_SERVER = "pool.ntp.org";
const char* API_HOST = "nuc.wouterds.be";
const int API_PORT = 443;

WiFiClientSecure client;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER);

void setup() {
  Serial.begin(9600);
  Serial.println();

  setupDisplay();
  setupWiFi();
  timeClient.begin();

  client.setInsecure();
}

void loop() {
  timeClient.update();
  JSONVar data = getData();

  updateDisplay(data);
}

void setupDisplay() {
  while (!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
    delay(10);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.cp437(true);
}

void setupWiFi() {
  display.clearDisplay();
  displayHeader("Setup");
  display.print("[WiFi] Connecting");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    display.print(".");
    display.display();
  }

  display.println("\n[WiFi] Connected");
  display.print("[WiFi] IP ");
  display.println(WiFi.localIP().toString());
  display.display();
  delay(2000);
}

JSONVar getData() {
  if (!client.connect(API_HOST, API_PORT)) {
    Serial.println("Could not connect to API");
    return JSON.parse("null");
  }

  client.println("GET https://" + String(API_HOST) + "/stats HTTP/1.0");
  client.println("Host: " + String(API_HOST));
  client.println("Connection: close");
  client.println();

  String response = client.readString();
  client.stop();

  int separatorIndex = response.indexOf("\r\n\r\n");
  if (separatorIndex != -1) {
    return JSON.parse(response.substring(separatorIndex + 4));
  }

  return JSON.parse("null");
}

void updateDisplay(JSONVar& data) {
  display.clearDisplay();
  displayHeader("NUC");
  displayDataWithProgressBar("CPU", data["cpu"], 17);
  displayDataWithProgressBar("Memory", data["memory"], 33);
  displayDataWithProgressBar("Disk", data["disk"], 49);
  display.display();
}

void displayDataWithProgressBar(const char* label, int percentage, int yPosition) {
  display.setCursor(0, yPosition);
  display.print(label);
  display.print(" ");
  display.print(percentage);
  display.print("%");

  drawProgressBar(percentage, 0, yPosition + 9, DISPLAY_WIDTH, 4);
}

void drawProgressBar(int percentage, int x, int y, int width, int height) {
  int filledWidth = (percentage * width) / 100;
  display.drawRect(x, y, width, height, SSD1306_WHITE);
  display.fillRect(x, y, filledWidth, height, SSD1306_WHITE);
}

void displayHeader(String title) {
  display.setCursor(0, 4);
  display.print(title);

  String time = getFormattedTime();
  int16_t timeWidth = time.length() * 6;
  display.setCursor(DISPLAY_WIDTH - timeWidth, 4);
  display.print(time);

  display.drawLine(0, 15, display.width() - 1, 15, SSD1306_WHITE);
  display.setCursor(0, 16);
}

String getFormattedTime() {
  return timeClient.getFormattedTime();
}
