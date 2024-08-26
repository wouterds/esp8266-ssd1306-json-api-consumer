#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "env.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDRESS 0x3C

WiFiClientSecure client;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

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

  display.clearDisplay();
  displayHeader("NUC SYSTEM");

  display.setCursor(0, 18);
  display.print("CPU ");
  display.print(data["cpu_used"]);
  display.print("%");
  display.display();

  display.setCursor(0, 27);
  display.print("RAM ");
  display.print(data["ram_used"]);
  display.print("%");
  display.display();

  display.setCursor(0, 36);
  display.print("Disk ");
  display.print(data["disk_used"]);
  display.print("%");
  display.display();

  delay(5000);
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

void setupWiFi()  {
  display.clearDisplay();
  displayHeader("Setup");
  display.print("[WiFi] Connecting");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    display.print(".");
    display.display();
  }
  delay(1000);

  display.println("");
  display.println("[WiFi] Connected");
  display.display();
  delay(1000);

  display.print("[WiFi] IP ");
  display.print(WiFi.localIP());
  display.println("");
  display.display();
  delay(1000);
}

JSONVar getData() {
  if (!client.connect("nuc.wouterds.be", 443)) {
    Serial.println("Could not connect to nuc.wouterds.be");
    return JSON.parse("null");
  }

  while (client.connected()) {
    client.println("GET https://nuc.wouterds.be/stats HTTP/1.0");
    client.println("Host: nuc.wouterds.be");
    client.println("Connection: close");
    client.println();

    String response = client.readString();
    client.stop();

    String headers = "";
    String body = "";
    int separatorIndex = response.indexOf("\r\n\r\n");
    if (separatorIndex != -1) {
        headers = response.substring(0, separatorIndex);
        body = response.substring(separatorIndex + 4);
    }

    return JSON.parse(body);
  }

  return JSON.parse("null");
}

String getFormattedTime() {
  String formattedTime = timeClient.getFormattedTime();
  return formattedTime;
}

void displayHeader(String title) {
  display.clearDisplay();
  display.setCursor(0, 4);
  display.print(title);

  String time = getFormattedTime();
  int16_t timeWidth = time.length() * 6;
  display.setCursor(DISPLAY_WIDTH - timeWidth, 4);
  display.print(time);

  display.drawLine(0, 15, display.width() - 1, 15, SSD1306_WHITE);
  display.setCursor(0, 16);
}
