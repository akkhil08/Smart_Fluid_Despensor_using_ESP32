#include "BluetoothSerial.h"

BluetoothSerial BT;  // Create an instance of the BluetoothSerial class

#define LED_PIN 33  // Define GPIO 2 for the LED

void setup() {
  Serial.begin(115200);  // Start Serial Monitor
  BT.begin("ESP32_LED_Control");  // Set Bluetooth name for ESP32
  Serial.println("Bluetooth is ready to pair!");

  pinMode(LED_PIN, OUTPUT);  // Initialize GPIO 2 as an output for the LED
  digitalWrite(LED_PIN, LOW);  // Start with the LED off
}

void loop() {
  // Check if Bluetooth has received data
  if (BT.available()) {
    String receivedData = BT.readStringUntil('\n');  // Read the incoming data until newline character
    receivedData.trim();  // Remove whitespace or newlines

    // Print received data to Serial Monitor
    Serial.print("Received: ");
    Serial.println(receivedData);

    // Control LED based on received data
    if (receivedData == "ON") {
      digitalWrite(LED_PIN, HIGH);  // Turn LED on
      Serial.println("LED turned ON");
      BT.println("LED turned ON");
    } 
    else if (receivedData == "OFF") {
      digitalWrite(LED_PIN, LOW);  // Turn LED off
      Serial.println("LED turned OFF");
      BT.println("LED turned OFF");
    }
  }
}

