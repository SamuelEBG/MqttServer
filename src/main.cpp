// Simplified example using arduino-mqtt by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <bits/stdc++.h>

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

WiFiClient networkClient;
MQTTClient mqttClient;

unsigned long lastMillis = 0;
unsigned long lastMillisTwo = 0;

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

String getRandomCity(const String cities[], const int sizeOfArray) {
    srand(time(NULL));  // Initialize random seed

    int randomIndex = rand() % sizeOfArray;  // Generate a random index between 0 and n-1

    return cities[randomIndex];
}

  String tempAsString;
  float temp;
  float feelsLike;
  String weatherMain;
  String weatherDesc;
  String country;
  String city;
  String locationFromApi;

void loop() {
    // Weather API
    String location = getRandomCity(cities, amountOfCities);
    String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + location + ",no&APPID=";
    const String key = "0fbfad07ebd8ba42e4384e55c8a77c58";
    const String units = "&units=metric";

    if(millis() - lastMillisTwo > 10000){
    lastMillisTwo = millis();
    
    HTTPClient http;

    http.useHTTP10(true);

    http.begin(endpoint + key + units); //HTTP, GET weather from API

    int httpCode = http.GET();

    if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.println("");
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          // String payload = http.getString();

          DynamicJsonDocument doc(1024);
          //StaticJsonDocument<200> doc;
          // StaticJsonDocument<N> allocates memory on the stack, it can be
          // replaced by DynamicJsonDocument which allocates in the heap.
          //
          // DynamicJsonDocument doc(200);

          // Deserialize the JSON document using getStream instead of getString
          DeserializationError error = deserializeJson(doc, http.getStream());

          // Test if parsing succeeds.
          if (error) {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              return;
          }

          // Fetch values.
          //
          // Most of the time, you can rely on the implicit casts.
          // In other case, you can do doc["time"].as<long>();
          //const char* name = doc["name"];
          String tempString = doc["main"]["temp"];
          tempAsString = tempString;
          temp = doc["main"]["temp"];
          feelsLike = doc["main"]["feels_like"];
          String weatherM = doc["weather"][0]["main"];
          weatherMain = weatherM;
          String weatherD = doc["weather"][0]["description"];
          weatherDesc = weatherD;
          String c = doc["sys"]["country"];
          country = c;
          String ci = doc["name"];
          city = ci;
          locationFromApi = city + ", " + country;

          // Change led according to the weather
          if (weatherMain.equalsIgnoreCase("clouds"))
          {
              strip.setPixelColor(0, 0,0,255);
              strip.show(); // Update strip with new contents
          }
          
          if (weatherMain.equalsIgnoreCase("sunny"))
          {
              strip.setPixelColor(0, 255,255,0);
              strip.show(); 
          }

          if (weatherMain.equalsIgnoreCase("snow"))
          {
              strip.setPixelColor(0, 255,255,255);
              strip.show();
          }

          if (weatherMain.equalsIgnoreCase("rain"))
          {
              strip.setPixelColor(0, 0, 255, 0);
              strip.show();
          }
          
          // Prepare payload to post to dweet
          // "{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}"
          /*
          String postData = "{\"temperature\":\""; 
          postData += weatherMain;
          postData += "\",\"location\":\"";
          postData += location;
          postData += "\"}";
          httpPost.addHeader("Content-Type", contentType);
          
          // send the POST request
          // read the status code and body of the response
          int statusCode = httpPost.POST(postData);

          Serial.print("Post status code: ");
          Serial.println(statusCode);
          */
          
          // Print values from GET to the terminal
          /*
          Serial.println(location);
          Serial.println(weatherMain);
          Serial.println(weatherDesc);
          Serial.printf("Temperature: ");
          Serial.println(temp);
          Serial.printf("Feels like: ");
          Serial.println(feelsLike);
          */
          
          //mqttClient.publish(yourPersonalTopic.c_str(), "Temperate is " + tempAsString + " degrees and " + weatherDesc + " in " + location);

          /*
          if (millis() - lastMillis > 5000) {
              lastMillis = millis();
              mqttClient.publish(yourPersonalTopic.c_str(), "Temperate is" + tempAsString + " degrees and " + weatherDesc + " in " + location);
          }
          */
      }
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    }

    mqttClient.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!mqttClient.connected()) {
        USE_SERIAL.printf("connecting again \n");
        connect();
    }
    
    // publish a message every 5 second.
    
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
      USE_SERIAL.printf("publishing\n");
      mqttClient.publish(yourPersonalTopic.c_str(), "Temperate is " + tempAsString + " degrees and " + weatherDesc + " in " + locationFromApi);
    }

}