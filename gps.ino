#include <TinyGPS++.h>

// Define the RX and TX pins for Serial 2
#define RXD2 3
#define TXD2 17

// Define pins for ultrasonic sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

#define GPS_BAUD 9600

// Define the height of the bin in cm
#define BIN_HEIGHT 100.0

// The TinyGPS++ object
TinyGPSPlus gps;

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial gpsSerial(2);

void setup() {
  // Serial Monitor
  Serial.begin(115200);

  // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");

  // Set up ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
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

      // Calculate the percentage filled
      double percentageFilled = ((BIN_HEIGHT - distance) / BIN_HEIGHT) * 100.0;

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
      Serial.print(percentageFilled, 2);
      Serial.println("%");

      // Check if the bin is full
      if (percentageFilled > 75.0) {
        Serial.println("Full");
      }

      Serial.println("");
    }
  }
}
