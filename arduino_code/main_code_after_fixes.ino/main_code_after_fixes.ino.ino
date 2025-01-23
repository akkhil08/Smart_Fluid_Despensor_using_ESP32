#include <WiFi.h>  // For ESP32
//#include <ESP8266WiFi.h>  // Uncomment for ESP8266

// WiFi Credentials
const char* ssid = "Raghuram's A15";      
const char* password = "sqpu7168";

double source = 230; // Available liquid source in mL

// Create WiFi Server on port 80
WiFiServer server(80);

// Pin Definitions
#define PUMP_PIN  18    // Relay for pump
#define FLOW_SENSOR_PIN 4 // Flow sensor input
#define PULSES_PER_LITER 63120  // Adjust based on sensor datasheet

// Global Variables
volatile uint64_t pulseCount = 0; 
unsigned long totalPulses = 0;
double totalLiters = 0.0;
bool pumpRunning = false;

// Interrupt Service Routine (ISR) for flow sensor
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// WiFi Reconnect Function
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Reconnecting WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nReconnected! IP address: " + WiFi.localIP().toString());
  }
}

// Function to start pump and fill liquid
void startFilling(WiFiClient& client, double targetVolume) {
  // if (source < targetVolume) { // Check if enough liquid is available
  //   Serial.println("Not enough liquid in source!");
  //   client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
  //   client.print("Not enough liquid in source!");
  //   return;
  // }

  totalLiters = 0.0;
  totalPulses = 0;
  pulseCount = 0;
  pumpRunning = true;
  digitalWrite(PUMP_PIN, HIGH); // Start Pump
  Serial.println("Pump Started");

  unsigned long lastTime = millis();
  unsigned long clientTime = millis();
  
  while (totalLiters < targetVolume / 1000.0 && pumpRunning) { // Convert mL to Liters
    checkWiFi();  // Ensure WiFi is connected

    unsigned long currentTime = millis();
    if (currentTime - lastTime >= 100) { // Every 100ms, update values
      totalLiters += (double)pulseCount / PULSES_PER_LITER;
      totalPulses += pulseCount;
      pulseCount = 0; // Reset pulse count for next interval

      // Send response in correct format: "source=xxx,sink=xxx"
    //   client.println(String(source - (totalLiters * 1000)) + "," + String(totalLiters * 1000));
    //   client.println("Connection: close");
    // client.println();
      // Send final response
    // client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");

      Serial.println("source=" + String(source - (totalLiters * 1000)) + ",sink=" + String(totalLiters * 1000));
      lastTime = currentTime;
    }
    if(millis() - clientTime > 800){
      client.print( String(source) + "," + String(totalLiters * 1000));
      client.print("\r\n");
      clientTime = millis();
    }
  }

  digitalWrite(PUMP_PIN, LOW); // Stop Pump
  pumpRunning = false;
  source -= totalLiters * 1000; // Update source after filling
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
  checkWiFi(); // Ensure WiFi remains connected

  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    //Serial.println(request);
    client.flush();

    // Parse the request
    if (request.indexOf("/start?") != -1) {
      int volumeIndex = request.indexOf("volume=");
      if (volumeIndex != -1) {
        String volumeString = request.substring(volumeIndex + 7);
        volumeString.trim();
        double volume = volumeString.toDouble();
        if (volume > 0) {
          client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
          //client.print("Filling " + String(volume) + " mL");
          startFilling(client, volume);  // Pass client reference
        } else {
          client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
          client.print("Invalid Volume");
        }
      } else {
        client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
        client.print("Volume Not Provided");
      }
    } else if (request.indexOf("/stop") != -1) {
      digitalWrite(PUMP_PIN, LOW);
      pumpRunning = false;
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Pump Stopped");
    } else if (request.indexOf("/reset") != -1) {
      totalLiters = 0.0;
      totalPulses = 0;
      source = 230; // Reset source value
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Reset done. Source refilled to 230 mL");
    } else {
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Liquid Filler System Ready");
    }

    client.stop();
  }
}
