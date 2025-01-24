#include <WiFi.h>

// WiFi Credentials
const char* ssid = "Raghuram's A15";
const char* password = "sqpu7168";

// Create WiFi Server on port 80
WiFiServer server(80);

// Pin Definitions  
#define PUMP_PIN 40    // Relay for pump
#define FLOW_SENSOR_PIN 4 // Flow sensor input
#define PULSES_PER_LITER 114750  // Adjust based on sensor datasheet

// Global Variables
volatile unsigned long pulseCount = 0;  // Prevent overflow
unsigned long totalPulses = 0;
double totalLiters = 0.0;
double source = 230; // Available liquid source in mL
double sink=0;
bool pumpRunning = false;

// Flag to track when data changes

double progressPercentage = 0.0; // Added to track filling progress

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
void startFilling(double targetVolume) {

  totalLiters = 0.0;
  totalPulses = 0;
  pulseCount = 0;
  pumpRunning = true;
  digitalWrite(PUMP_PIN, HIGH); // Start Pump
  Serial.println("Pump Started");

  unsigned long startTime = millis();

  while (totalLiters < targetVolume / 1000.0 && pumpRunning) {
    checkWiFi();

    // Timeout safeguard (Stop pump if running too long)
    if (millis() - startTime > 15000) {  
      Serial.println("Timeout! Stopping pump.");
      break;
    }

    unsigned long currentTime = millis();
    if (currentTime - startTime >= 100) { // Update values every 100ms
      totalLiters += (double)pulseCount / PULSES_PER_LITER;
      totalPulses += pulseCount;
      pulseCount = 0; // Reset pulse count for next interval

      // Calculate progress percentage
      progressPercentage = (totalLiters * 1000 / targetVolume) * 100;

      // Set flag to indicate data update


      Serial.println("Progress: " + String(progressPercentage) + "%");
      startTime = currentTime;
    }
  }

  digitalWrite(PUMP_PIN, LOW); // Stop Pump
  pumpRunning = false;
  source -= totalLiters * 1000; // Update source after filling
  Serial.println("Pump Stopped");

  // Final data update
}

// Handle HTTP Requests
void handleRequest(WiFiClient& client) {
  String request = "";
  while (client.available()) {
    request += client.readStringUntil('\n');  // Read full request
  }

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
        // client.print("Filling " + String(volume) + " mL");
        client.print("source=" + String(source - (totalLiters*1000)) + "," + "sink=" + String(totalLiters*1000));

        startFilling(volume);
      } else {
        client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
        client.print("Invalid Volume");
      }
    } else {
      client.print("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Volume Not Provided");
    }
  } 
  else if (request.indexOf("/reset") != -1) {
    totalLiters = 0.0;
    totalPulses = 0;
    source = 230; // Reset source value
    sink=0;
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    client.print("Reset done. Source refilled to 230 mL");
  } 
  else if (request.indexOf("/progress") != -1) { // Added endpoint for progress updates
    String jsonResponse = "{";
    jsonResponse += "\"source\": " + String(source, 2) + ","; // Include source with two decimal points
    jsonResponse += "\"sink\": " + String(totalLiters * 1000, 2) + ","; // Include sink in mL with two decimal points
    jsonResponse += "\"progress\": " + String(progressPercentage, 2); // Include progress percentage
    jsonResponse += "}";

    client.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
    client.print(jsonResponse);
    Serial.println(jsonResponse);
  } 
  else {
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    client.print("Liquid Filler System Ready");
  }

  client.stop(); // End the client connection
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
  checkWiFi();

  WiFiClient client = server.available();
  if (client) {
    handleRequest(client);  // Handle the incoming client request
  }
}
