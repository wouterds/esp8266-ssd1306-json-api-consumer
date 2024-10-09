#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "env.h"

const int DISPLAY_WIDTH = 128;
const int DISPLAY_HEIGHT = 64;
const int DISPLAY_ADDRESS = 0x3C;
const char* NTP_SERVER = "pool.ntp.org";
const char* API_HOST = "nuc.wouterds.be";
const int API_PORT = 443;

static WiFiClientSecure client;
static Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, NTP_SERVER);

const unsigned char thermometerIcon[] PROGMEM = {
  0b00100000,
  0b01010000,
  0b01010000,
  0b01010000,
  0b01010000,
  0b01010000,
  0b10001000,
  0b11111000,
  0b11111000,
  0b01110000
};

struct Stats {
  float cpu;
  int cpu_temp;
  float memory;
  float disk;
};

void displayHeader(String title, int temperature = -1);
void drawProgressBar(int percentage, int x, int y, int width, int height);
void displayDataWithProgressBar(const char* label, float value, int yPosition);

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
  Stats data = getData();
  updateDisplay(data);
  delay(100);
}

void setupDisplay() {
  while (!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
    delay(25);
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
  display.setCursor(0, 18);
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

Stats getData() {
  Stats stats = {0, 0, 0, 0};

  if (!client.connect(API_HOST, API_PORT)) {
    Serial.println(F("Could not connect to API"));
    return stats;
  }

  client.println(F("GET /stats HTTP/1.1"));
  client.print(F("Host: "));
  client.println(API_HOST);
  client.println(F("Connection: close"));
  client.println();

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, client);
  client.stop();

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return stats;
  }

  stats.cpu = doc["cpu"].as<float>();
  stats.cpu_temp = doc["cpu_temp"].as<int>();
  stats.memory = doc["memory"].as<float>();
  stats.disk = doc["disk"].as<float>();

  return stats;
}

void updateDisplay(const Stats& data) {
  display.clearDisplay();
  displayHeader("NUC", data.cpu_temp);
  displayDataWithProgressBar("CPU", data.cpu, 18);
  displayDataWithProgressBar("Memory", data.memory, 34);
  displayDataWithProgressBar("Disk", data.disk, 50);
  display.display();
}

void displayDataWithProgressBar(const char* label, float value, int yPosition) {
  display.setCursor(0, yPosition);
  display.print(label);

  char percentStr[6];
  snprintf(percentStr, sizeof(percentStr), "%.1f%%", value);

  display.setCursor(DISPLAY_WIDTH - strlen(percentStr) * 6, yPosition);
  display.print(percentStr);

  drawProgressBar(int(value), 0, yPosition + 9, DISPLAY_WIDTH, 4);
}

void drawProgressBar(int percentage, int x, int y, int width, int height) {
  int filledWidth = (percentage * width) / 100;
  display.drawRect(x, y, width, height, SSD1306_WHITE);
  display.fillRect(x, y, filledWidth, height, SSD1306_WHITE);
}

void displayHeader(String title, int temperature) {
  display.setCursor(0, 5);
  display.print(title);

  if (temperature != -1) {
    display.drawBitmap(25, 3, thermometerIcon, 8, 10, SSD1306_WHITE);
    display.setCursor(34, 5);
    display.print(String(temperature));
    display.drawCircle(display.getCursorX() + 2, 5, 1, SSD1306_WHITE);
    display.setCursor(display.getCursorX() + 5, 5);
    display.print("C");
  }

  String time = getFormattedTime();
  int16_t timeWidth = time.length() * 6;
  display.setCursor(DISPLAY_WIDTH - timeWidth, 5);
  display.print(time);

  display.drawLine(0, 15, display.width() - 1, 15, SSD1306_WHITE);
  display.setCursor(0, 16);
}

String getFormattedTime() {
  static char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());
  return String(timeStr);
}
