#include <HardwareSerial.h>

// Use hardware serial (UART1) for Bluetooth communication
HardwareSerial Bluetooth(1);  // UART1 for Bluetooth (you can choose UART2, or use another pin)

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
  Bluetooth.print("Starting calibration for ");
  Bluetooth.print(targetVolume);
  Bluetooth.println(" mL...");

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
      Bluetooth.print("Flow Rate (L/min): ");
      Bluetooth.println(flowRate);
      Bluetooth.print("Total Pulses: ");
      Bluetooth.println(totalPulseCount);
      Bluetooth.print("Total Liters: ");
      Bluetooth.println(totalLiters * 1000);  // Convert Liters back to mL for display
    }
  }

  // Stop the pump after reaching the target volume
  digitalWrite(RELAY_PIN, LOW);
  Bluetooth.println("Calibration complete.");
  Bluetooth.print("Total Pulses: ");
  Bluetooth.println(totalPulseCount);
  Bluetooth.print("Total Liters: ");
  Bluetooth.println(totalLiters);
}

// Setup function
void setup() {
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);

  // Attach the interrupt to the flow sensor pin
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  // Initialize hardware serial communication for Bluetooth (using UART1)
  Bluetooth.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17 for UART1
  Serial.begin(115200); // For debugging, using default Serial (USB)
  
  Serial.println("Flow sensor system initialized.");
  Bluetooth.println("Flow sensor system initialized.");
}

// Main loop
void loop() {
  if (Bluetooth.available()) {
    String receivedData = Bluetooth.readStringUntil('\n');
    receivedData.trim();  // Remove any leading/trailing spaces or newline characters

    if (receivedData == "50") {
      calibrate(50); // Start calibration for 50 mL
    } else if (receivedData == "100") {
      calibrate(100); // Start calibration for 100 mL
    } else if (receivedData.toInt() > 0) {
      double customVolume = receivedData.toInt();  // Custom volume (in mL)
      calibrate(customVolume);
    } else {
      Bluetooth.println("Invalid command. Send '50', '100', or a custom value in mL.");
    }
  }

  delay(100); // Small delay to prevent busy looping
}
