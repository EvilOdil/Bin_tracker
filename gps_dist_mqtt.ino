#include <TinyGPS++.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Define the RX and TX pins for Serial 2
#define RXD2 3
#define TXD2 17

// Define pins for ultrasonic sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

#define GPS_BAUD 9600

// Define the height of the bin in cm
#define BIN_HEIGHT 100.0

// WiFi and MQTT configurations
const char* ssid = "Odil";
const char* password = "odil1234";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
//const char* mqttUser = "your_USERNAME";
//const char* mqttPassword = "your_PASSWORD";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// The TinyGPS++ object
TinyGPSPlus gps;

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial gpsSerial(2);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  // Serial Monitor
  Serial.begin(115200);

  // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");

  // Set up ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Set up MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed MQTT connection, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
  mqttClient.subscribe("bin/control");

}

// Function to measure distance using ultrasonic sensor
double measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  double distance = (duration * 0.0343) / 2; // Convert to cm

  return distance;
}

void loop() {
  // This sketch displays information every time a new sentence is correctly encoded.
  unsigned long start = millis();

  while (millis() - start < 1000) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      // Measure distance
      double distance = measureDistance();

      // Calculate the percentage filled and round to the nearest multiple of 5
      double percentageFilled = ((BIN_HEIGHT - distance) / BIN_HEIGHT) * 100.0;
      percentageFilled = round(percentageFilled / 5.0) * 5.0;

      // Prepare MQTT topics
      String latTopic = "bin/latitude";
      String lngTopic = "bin/longitude";
      String fillTopic = "bin/percentage_filled";
      String fullTopic = "bin/status";

      // Publish latitude
      String latitude = String(gps.location.lat(), 6);
      mqttClient.publish(latTopic.c_str(), latitude.c_str());

      // Publish longitude
      String longitude = String(gps.location.lng(), 6);
      mqttClient.publish(lngTopic.c_str(), longitude.c_str());

      // Publish percentage filled
      String percentage = String(percentageFilled, 0);
      mqttClient.publish(fillTopic.c_str(), percentage.c_str());

      // Check if the bin is full and publish status
      if (percentageFilled > 75.0) {
        mqttClient.publish(fullTopic.c_str(), "Full");
        Serial.println("Full");
      }

      // Print GPS and distance information
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: "); 
      Serial.println(gps.location.lng(), 6);
      Serial.print("Time in UTC: ");
      Serial.println(String(gps.date.year()) + "/" + String(gps.date.month()) + "/" + String(gps.date.day()) + "," + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()));
      Serial.print("Distance (cm): ");
      Serial.println(distance, 2);
      Serial.print("Percentage Filled: ");
      Serial.print(percentageFilled, 0);
      Serial.println("%");

      Serial.println("");
    }
  }

  // Ensure the MQTT client stays connected
  mqttClient.loop();
}
