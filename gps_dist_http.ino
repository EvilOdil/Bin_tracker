#include <TinyGPS++.h>
#include <WiFi.h>
#include <WebServer.h>

// Define the RX and TX pins for Serial 2
#define RXD2 3
#define TXD2 17

// Define pins for ultrasonic sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

#define GPS_BAUD 9600

// Define the height of the bin in cm
#define BIN_HEIGHT 100.0

// WiFi configurations
const char* ssid = "Odil";
const char* password = "odil1234";

WebServer server(80);

// The TinyGPS++ object
TinyGPSPlus gps;

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial gpsSerial(2);

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

void handleData() {
  // Measure distance
  double distance = measureDistance();

  // Calculate the percentage filled and round to the nearest multiple of 5
  double percentageFilled = ((BIN_HEIGHT - distance) / BIN_HEIGHT) * 100.0;
  percentageFilled = round(percentageFilled / 5.0) * 5.0;

  // Prepare GPS data
  String latitude = gps.location.isValid() ? String(gps.location.lat(), 6) : "N/A";
  String longitude = gps.location.isValid() ? String(gps.location.lng(), 6) : "N/A";
  String percentage = String(percentageFilled, 0);
  String status = percentageFilled > 75.0 ? "Full" : "Not Full";

  // Create JSON response
  String json = "{";
  json += "\"latitude\": \"" + latitude + "\",";
  json += "\"longitude\": \"" + longitude + "\",";
  json += "\"percentage_filled\": \"" + percentage + "\",";
  json += "\"status\": \"" + status + "\"";
  json += "}";

  server.send(200, "application/json", json);
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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up HTTP server
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Process GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Handle HTTP requests
  server.handleClient();
}
