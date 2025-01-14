#define RELAY_PIN 16 // ESP32 pin GPIO16, which connects to the pump the via the relay
// constexpr int PULSES_PER_100ML = 8235; // flow sensor pulses per 100ml
constexpr int PULSES_PER_LITER = 93988; // Replace with your flow sensor's pulses per liter value (from the datasheet)
int pp=0;

int FLOW_SENSOR_PIN = 14;    //This is the input pin on the Arduino
double flowRate;    //This is the value we intend to calculate.
volatile unsigned long pulseCount = 0;
unsigned long lastTime = 0;
float totalLiters = 0;
// the setup function runs once when you press reset or power the board

void calibrate();


// Interrupt Service Routine (ISR) to count pulses
void IRAM_ATTR pulseCounter() {
  pulseCount++;
  pp++;
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

// the loop function runs over and over again forever
void loop() {
  // digitalWrite(RELAY_PIN, HIGH); // turn on pump 4 seconds
  //count = 0;      // Reset the counter so we start counting from 0 again

  unsigned long currentTime = millis();

  calibrate();



  // // Calculate flow rate every second
  // if (currentTime - lastTime >= 1000) {
  //   // Disable interrupts temporarily to safely read pulseCount
  //   detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
  //   unsigned long pulses = pulseCount;
  //   pulseCount = 0; // Reset pulse count
  //   attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
  //   // Serial.println(pulses);
  //   flowRate = ((float)pulses / PULSES_PER_LITER) * 60;

  //   // Update total liters
  //   totalLiters += (float)pulses / PULSES_PER_LITER;
    
  //   Serial.print("Flow rate: ");
  //   Serial.print(flowRate);
  //   Serial.println(" L/min");
  //   Serial.print("totalPulse = ");
  //   Serial.println(pp);
  //   Serial.print("Total volume: ");
  //   Serial.print(totalLiters);
  //   Serial.println(" L");

    // lastTime = currentTime;

  // }

}

void calibrate(){
  totalLiters = 0;
  char input = 0;
  
  Serial.print("Press to 'y' start test:");
  while (input != 'y') { 
    input = (char)Serial.read();
  }

  Serial.print("Test running ..."); 
  digitalWrite(RELAY_PIN, HIGH); // turn on pump 4 seconds
  lastTime = millis();
  
  while(totalLiters <= 0.15){
    unsigned long currentTime = millis();
    if (currentTime - lastTime >= 100) {
      detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN)); // Disable interrupts temporarily to safely read pulseCount
      flowRate = ((float)pulseCount / PULSES_PER_LITER) * 60;
      totalLiters += (float)pulseCount / static_cast<float>(PULSES_PER_LITER);  // Update total liters
      pp++;
      Serial.print("Pulses: ");
      Serial.println(pulseCount);
      Serial.print("Total Pulses: ");
      Serial.println(pp);
      Serial.print("total litres: ");
      Serial.println(totalLiters);
      pulseCount = 0; // Reset pulse count
      attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
      lastTime = currentTime;
    }
  }
  pp=0;
  digitalWrite(RELAY_PIN, false);
}

// void fillUpto(float )
