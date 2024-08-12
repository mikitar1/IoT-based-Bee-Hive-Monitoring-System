#include <Arduino.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#include "HX711.h"

// DHT Sensor
#define DHTPIN A0          
#define DHTTYPE DHT11      
DHT dht(DHTPIN, DHTTYPE);
// Load Cell
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
HX711 scale;

// PIR Sensor
const int ledPin = 8;
const int inputPin = 4;
int pirState = LOW;
int val = 0;

// GSM Module
SoftwareSerial gsmSerial(6, 7); // RX, TX

// Function Prototype
void sendSMS(const char* message);

// Calibration constant for load cell
float calibration_factor = 36.059;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(inputPin, INPUT);
  Serial.begin(9600);
  gsmSerial.begin(9600);
  dht.begin();
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println(scale.get_units(), 1);  
  scale.set_scale(calibration_factor);
  scale.tare();
}

void loop() {
  delay(2000);

  // Read DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" *C ");
    Serial.println();
    
    if (temperature > 30 || temperature < 15) {
      sendSMS("Abnormal temperature");
    }
    if (humidity < 70) {
      sendSMS("Abnormal humidity");
    }
  }

  // Read Load Cell
  float weight = scale.get_units(5) / 1000;
  Serial.print("Weight in KG: ");
  Serial.println(weight, 1);
  if (weight > 25) {
    sendSMS("Full of honey");
  }

  // Read PIR Sensor
  val = digitalRead(inputPin);
  if (val == HIGH) {
    digitalWrite(ledPin, HIGH);
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      sendSMS("Motion detected");
      pirState = HIGH;
    }
  } else {
    digitalWrite(ledPin, LOW);
    if (pirState == HIGH){
      Serial.println("Motion ended!");
      pirState = LOW;
    }
  }
}
void sendSMS(const char* message) {
  gsmSerial.print("AT+CMGF=1\r");   
  delay(1000);
  gsmSerial.print("AT+CMGS=\"+251949055321\"\r"); 
  delay(1000);
  gsmSerial.print(message);      
  gsmSerial.write(26);           
  Serial.print("Sent SMS: ");    
  Serial.println(message);
}
