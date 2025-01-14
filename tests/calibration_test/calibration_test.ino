#define FLOW_SENSOR_PIN 14      //This is the input pin on the Arduino
#define RELAY_PIN 16            // ESP32 pin GPIO16, which connects to the pump the via the relay
#define PULSES_PER_LITER 93988  // Replace with your flow sensor's pulses per liter value (from the datasheet)

volatile unsigned long pulseCount = 0;

// Interrupt Service Routine (ISR) to count pulses
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}


void calibrate(){
  double totalLiters = 0;
  char input = 0;
  
  // Start test
  Serial.print("Press to 'y' start test:");
  while (input != 'y') { 
    input = (char)Serial.read();
  }

  // Running test
  Serial.print("Test running ..."); 
  digitalWrite(RELAY_PIN, HIGH); // turn on pump 4 seconds
  unsigned long lastTime = millis();
  long unsigned int totalPulseCount = 0;

  while(totalLiters <= 0.15){   // Run until 150ml fluid is transfered
    unsigned long currentTime = millis();
    if (currentTime - lastTime >= 100) {
      detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN)); // Disable interrupts temporarily to safely read pulseCount
      double flowRate = ((float)pulseCount / PULSES_PER_LITER) * 60;
      totalLiters += (float)pulseCount / static_cast<float>(PULSES_PER_LITER);  // Update total liters
      totalPulseCount +=pulseCount;

      Serial.print("Total Pulses: ");
      Serial.println(totalPulseCount);
      Serial.print("total litres: ");
      Serial.println(totalLiters);
      
      // Reset
      pulseCount = 0;
      attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
      lastTime = currentTime;
    }
  }
  digitalWrite(RELAY_PIN, false);
}


void setup() {
  // initialize digital pin GPIO16 as an output.
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  
  // Attach the interrupt to the pin
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
  Serial.begin(9600);
  Serial.println("Flow sensor initialized.");
}


void loop() {
  calibrate();
}