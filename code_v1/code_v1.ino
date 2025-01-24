#include <WiFi.h>  // For ESP32
//#include <ESP8266WiFi.h>  // Uncomment for ESP8266

// WiFi Credentials
const char* ssid = "Raghuram's A15";      
const char* password = "sqpu7168";

unsigned int source = 300;  // Available liquid in mL
unsigned int sink = 0;      // Collected liquid in mL

// Create WiFi Server on port 80
WiFiServer server(80);

// Pin Definitions
#define PUMP_PIN  40           // Relay for pump
#define FLOW_SENSOR_PIN 4      // Flow sensor input
#define PULSES_PER_LITER 91500 // Adjust based on imperiacal data

// Global Variables
volatile unsigned long pulseCount = 0; 
unsigned long totalPulses = 0;
double totalLiters = 0.0;
bool pumpRunning = false; // Flag to track pump state

// Interrupt Service Routine (ISR) for flow sensor
void IRAM_ATTR pulseCounter() {
  if (pumpRunning) {
    pulseCount++;
  }
}

// Function to start pump and fill liquid
void startFilling(double targetVolume) {
  WiFiClient client = server.available();
  totalLiters = 0.0;
  totalPulses = 0;
  pulseCount = 0;
  pumpRunning = true;
  digitalWrite(PUMP_PIN, HIGH); // Start Pump
  
  unsigned long lastTime = millis();
  while (totalLiters < targetVolume / 1000.0) { // Convert mL to Liters
    unsigned long currentTime = millis();
    if (currentTime - lastTime >= 100) {
      detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
      totalLiters += (double)pulseCount / PULSES_PER_LITER;
      totalPulses += pulseCount;
      pulseCount = 0;
      attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
      Serial.println(String(totalLiters) + "," + String(totalPulses) + "," + String(targetVolume));
      lastTime = currentTime;
    }
  }

  // Update the source and sink values after filling
  source -= totalLiters * 1000;
  sink += totalLiters * 1000;
  
  stopFilling();  // Stop pump after filling
  client.print("source=" + String(source) + "," + "sink=" + String(sink));
}

// Function to stop filling
void stopFilling() {
  digitalWrite(PUMP_PIN, LOW); // Stop Pump
  pumpRunning = false;
  pulseCount = 0;
  totalLiters = 0.0;
  totalPulses = 0;
  Serial.println("Pump Stopped");
}

// Setup Function
void setup() {
  Serial.begin(115200);
  
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP address:");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Web Server Started");
}

// Main Loop
void loop() {
  WiFiClient client = server.available();

  if (client) {
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Parse the request
    if (request.indexOf("/start?") != -1) {
      int volumeIndex = request.indexOf("volume=");
      if (volumeIndex != -1) {
        String volumeString = request.substring(volumeIndex + 7);
        volumeString.trim();
        double volume = volumeString.toDouble();
        
        // Limit check: Only allow up to 250 mL
        if (volume > 0 && volume <= 250) {
          startFilling(volume);
          client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
          client.print("source=" + String(source) + "," + "sink=" + String(sink));
        } 
        else {
          client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
          client.print("Invalid Volume! Enter a value between 1-250 mL.");
        }
      } 
      else {
        client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
        client.print("Volume Not Provided");
      }
    } 
    else if (request.indexOf("/reset") != -1) {
      source = 300;
      sink = 0;
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("source=" + String(source) + "," + "sink=" + String(sink));
    } 
    else {
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Liquid Filler System Ready");
    }

    client.stop();
  }
}
