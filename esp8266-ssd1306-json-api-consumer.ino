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

  for (int i = 0; i < 6; i++) {
    displayTeslaData(data["tesla"]);
    delay(5000);

    displayAranetData(data["aranet"]);
    delay(5000);
  }
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
  display.setCursor(0, 0);
  display.println("    Tesla Model 3");
  display.drawLine(0, 9, display.width() - 1, 9, SSD1306_WHITE);

  display.setCursor(0, 14);
  display.print("Battery ");
  display.print(data["battery"]);
  display.print("%");
  display.display();

  display.setCursor(0, 24);
  display.print("Distance ");
  display.print(data["distance"]);
  display.print(" km");
  display.display();

  display.setCursor(0, 34);
  display.print("Wake ");
  display.print(data["wake"]);
  display.display();
}

void displayAranetData(JSONVar data) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("       Aranet 4");
  display.drawLine(0, 9, display.width() - 1, 9, SSD1306_WHITE);

  display.setCursor(0, 14);
  display.print("CO2 ");
  display.print(data["co2"]);
  display.print(" ppm");

  display.setCursor(0, 24);
  display.print("Temperature ");
  display.print(data["temperature"]);
  display.drawCircle(100, 24, 2, SSD1306_WHITE);
  display.setCursor(105, 24);
  display.print("C");
  display.display();

  display.setCursor(0, 34);
  display.print("Humidity ");
  display.print(data["humidity"]);
  display.print("%");
  display.display();

  display.setCursor(0, 44);
  display.print("Pressure ");
  display.print(data["pressure"]);
  display.print(" hPa");
  display.display();

  display.setCursor(0, 54);
  display.print("Battery ");
  display.print(data["battery"]);
  display.print("%");
  display.display();
}
