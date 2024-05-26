#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

const char* WIFI_SSID = "";
const char* WIFI_PASS = "";

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDRESS 0x3C

WiFiClientSecure client;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);

void setup() {
  Serial.begin(9600);
  Serial.println();

  Serial.println("[setup] display");
  while (!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
    delay(10);
  }

  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0, 0);
  display.cp437(true);
  delay(500);

  Serial.println("[setup] wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  display.print("[WiFi] Connecting");
  display.display();
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    display.print(".");
    display.display();
  }
  delay(500);
  
  display.println("");
  display.println("[WiFi] Connected");
  display.display();
  delay(1000);
  
  display.print("[WiFi] IP ");
  display.print(WiFi.localIP());
  display.println("");
  display.display();
  
  Serial.print("[setup] wifi: connected, ip: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  
  display.println("");
  display.println("Fetching data");
  display.display();

  client.setInsecure();
}

void loop() {
  Serial.println("connecting to wouterds.be");
  if (!client.connect("wouterds.be", 443)) {
    Serial.println("Could not connect with wouterds.be");
    return;
  }

  while (client.connected()) {
    client.println("GET https://wouterds.be/api/tesla HTTP/1.0");
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
    
    Serial.println("Headers:");
    Serial.println(headers);
    
    Serial.println("Body:");
    Serial.println(body);
    
    JSONVar data = JSON.parse(body);
    
    display.clearDisplay();
    display.setCursor(0, 1);
    display.println("    Tesla Model 3");
    display.drawLine(0, 11, display.width() - 1, 11, SSD1306_WHITE);

    display.setCursor(0, 18);
    display.print("Battery ");
    display.print(data["battery"]);
    display.print("%");

    display.setCursor(0, 28);
    display.print("Distance ");
    display.print(data["distance"]);
    display.print(" km");
    display.display();
  }

  delay(5000);
}
