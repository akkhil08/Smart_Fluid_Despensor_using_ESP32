#include "BluetoothSerial.h"
// Use hardware serial (UART1) for Bluetooth communication
BluetoothSerial BT;  // UART1 for Bluetooth (you can choose UART2, or use another pin)

#define FLOW_SENSOR_PIN 4      // Input pin for flow sensor
#define RELAY_PIN 18           // Output pin for pump relay
#define PULSES_PER_LITER 63120 // Pulses per liter from the datasheet

// Global Variables
volatile unsigned long pulseCount = 0; // Counts pulses from the flow sensor
unsigned long totalPulseCount = 0;     // Total pulse count across all tests
double totalLiters = 0.0;              // Total liters measured

// Interrupt Service Routine (ISR) to count pulses
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// Function: Calibrate the Flow Sensor and measure specific volume
void calibrate(double targetVolume) {
  totalLiters = 0.0;  // Reset total liters for this test
  totalPulseCount = 0; // Reset total pulse count for this test
  unsigned long lastTime = millis();

  // Notify Bluetooth client about the start of calibration
  BT.print("Starting calibration for ");
  BT.print(targetVolume);
  BT.println(" mL...");

  digitalWrite(RELAY_PIN, HIGH); // Turn on the pump

  // Loop until the desired volume (in liters) is transferred
  while (totalLiters < targetVolume / 1000.0) { // Convert target volume from mL to Liters
    unsigned long currentTime = millis();
    if (currentTime - lastTime >= 100) { // Update every 100ms
      // Disable interrupts to safely read pulseCount
      detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
      double flowRate = ((float)pulseCount / PULSES_PER_LITER) * 60; // Flow rate in liters per minute
      totalLiters += (float)pulseCount / static_cast<float>(PULSES_PER_LITER);  // Update total liters
      totalPulseCount += pulseCount;  // Update total pulses
      pulseCount = 0; // Reset pulse count
      attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING); // Re-enable interrupts
      lastTime = currentTime;

      // Send progress to Bluetooth
      BT.print("Flow Rate (L/min): ");
      BT.println(flowRate);
      BT.print("Total Pulses: ");
      BT.println(totalPulseCount);
      BT.print("Total Liters: ");
      BT.println(totalLiters * 1000);  // Convert Liters back to mL for display
    }
  }

  // Stop the pump after reaching the target volume
  digitalWrite(RELAY_PIN, LOW);
  BT.println("Calibration complete.");
  BT.print("Total Pulses: ");
  BT.println(totalPulseCount);
  BT.print("Total Liters: ");
  BT.println(totalLiters);
}

// Setup function
void setup() {
  Serial.begin(115200);  // Start Serial Monitor
  BT.begin("ESP32_Flow_Control");  // Set Bluetooth name for ESP32
  Serial.println("Bluetooth is ready to pair!");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

  // Attach the interrupt to the flow sensor pin
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  // Initialize the BluetoothSerial connection for communication with the user
  Serial.println("Flow sensor system initialized.");
  BT.println("Flow sensor system initialized.");
}

// Main loop
void loop() {
  if (BT.available()) {
    String receivedData = BT.readStringUntil('\n'); // Read the data until a newline
    receivedData.trim(); // Remove leading/trailing spaces or newlines

    // Convert received data to an integer
    double receivedValue = receivedData.toDouble(); // Convert to integer for comparison

    // Check if the value matches predefined volumes or is a custom valid value
    if (receivedValue == 50) {
      calibrate(50); // Start calibration for 50 mL
    } else if (receivedValue == 100) {
      calibrate(100); // Start calibration for 100 mL
    } else if (receivedValue > 0) { 
      calibrate(receivedValue); // Custom volume (in mL)
    } else {
      // If the input is invalid, notify the user via Bluetooth
      BT.println("Invalid command. Send '50', '100', or a custom positive value in mL.");
    }
  }

  delay(100); // Small delay to prevent busy looping
}
