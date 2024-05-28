#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include "env.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDRESS 0x3C

WiFiClientSecure client;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);

void setup() {
  Serial.begin(9600);
  Serial.println();

  setupDisplay();
  setupWiFi();

  client.setInsecure();
}

void loop() {
  JSONVar data = getData();

  displayTeslaData(data["tesla"]);
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
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  display.print("[WiFi] Connecting");
  display.display();

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
  if (!client.connect("wouterds.be", 443)) {
    Serial.println("Could not connect to wouterds.be");
    return JSON.parse("null");
  }

  while (client.connected()) {
    client.println("GET https://wouterds.be/api/experiments HTTP/1.0");
    client.println("Host: wouterds.be");
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

void displayTeslaData(JSONVar data) {
  display.clearDisplay();
  display.setCursor(0, 1);
  display.println("    Tesla Model 3");
  display.drawLine(0, 11, display.width() - 1, 11, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("Battery ");
  display.print(data["battery"]);
  display.print("%");

  display.setCursor(0, 29);
  display.print("Distance ");
  display.print(data["distance"]);
  display.print(" km");
  display.display();

  display.setCursor(0, 40);
  display.print("Wake ");
  display.print(data["wake"]);
  display.display();
}
