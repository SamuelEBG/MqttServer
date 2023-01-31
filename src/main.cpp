// Simplified example using arduino-mqtt by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <Arduino.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_DotStar.h>

#include <iostream>
#include <cstdlib>

// These are variables needed to use the LED on the ESP32S3 board
#define USE_SERIAL Serial
#define NUMPIXELS 1 // There is only one pixel on the board
// Use these pin definitions for the ItsyBitsy M4
#define DATAPIN    33
#define CLOCKPIN   21
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

//const char ssid[] = "MaxChillOutCrib";
//const char pass[] = "Ch1ll3rn!";
const char ssid[] = "Student";
const char pass[] = "Kristiania1914";
const char mqtt_username[] = "sago004";
const char mqtt_password[] = "KX1~C^4U1e8GMz4";
const char mqtt_server[]   = "mqtt.toytronics.com";
String yourPersonalTopic;

const String cities[] = {"Kristiansand", "Oslo", "Alta", "Harstad", "Molde", "Bergen", "Mo i Rana"};
const int amountOfCities = sizeof(cities) / sizeof(cities[0]);  // Number of elements in the array

String location = getRandomCity(cities, amountOfCities);
String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + location + ",no&APPID=";
const String key = "0fbfad07ebd8ba42e4384e55c8a77c58";
const String units = "&units=metric";

WiFiClient networkClient;
MQTTClient mqttClient;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  String clientId = "ESP8266Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);

  Serial.print("\nConnecting to Wifi...");
  while (!mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
    Serial.print(".");
    delay(1000);
  }

  //yourPersonalTopic = "kristiania/pgr212/color";
  yourPersonalTopic = "students/"; // Create a topic path based on your username
  yourPersonalTopic += String(mqtt_username);
  Serial.print("\nConnected to Wifi! Setting up Subscription to the topic: ");
  Serial.println( yourPersonalTopic );

  mqttClient.subscribe( yourPersonalTopic.c_str() );
}

// Note: Do not publish, subscribe or unsubscribe in this method
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming message: " + topic + " - " + payload);
}

void setup() {
  strip.begin(); // Initialize pins for output
  strip.setBrightness(30);
  strip.show();  // Turn all LEDs off ASAP

  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }

  WiFi.begin(ssid, pass);

  mqttClient.begin(mqtt_server, networkClient);
  mqttClient.onMessage(messageReceived);

  connect();
}

void loop() {
  mqttClient.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!mqttClient.connected()) {
    connect();
  }

  // publish a message every 5 second.
  /*
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    mqttClient.publish(yourPersonalTopic.c_str(), "hello");
  }
  */
}

String getRandomCity(const String cities[], const int sizeOfArray) {
    srand(time(NULL));  // Initialize random seed

    int randomIndex = rand() % sizeOfArray;  // Generate a random index between 0 and n-1

    return cities[randomIndex];
}