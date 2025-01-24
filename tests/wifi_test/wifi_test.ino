#include <WiFi.h>  // For ESP32
//#include <ESP8266WiFi.h>  // Uncomment for ESP8266

const char* ssid = "Raghuram's A15";       // Replace with your WiFi SSID
const char* password = "sqpu7168"; // Replace with your WiFi password

WiFiServer server(80);  // Start server on port 80

#define LED_PIN 2  // GPIO2 (Built-in LED on most ESP boards)

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Turn off LED initially

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());  // Get ESP IP Address

  server.begin();  // Start the web server
}

void loop() {
  WiFiClient client = server.available();  // Check for incoming client
  
  if (client) {
    Serial.println("Client connected");
    String request = client.readStringUntil('\r');  // Read HTTP request
    Serial.println(request);
    client.flush();

    // Check if request is for turning LED ON or OFF
    if (request.indexOf("/LED=ON") != -1) {
      digitalWrite(LED_PIN, HIGH);
    } else if (request.indexOf("/LED=OFF") != -1) {
      digitalWrite(LED_PIN, LOW);
    }

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<html><body>");
    client.println("<h1>ESP Web Server</h1>");
    client.println("<p>Click below to control the LED:</p>");
    client.println("<a href=\"/LED=ON\"><button>Turn ON</button></a>");
    client.println("<a href=\"/LED=OFF\"><button>Turn OFF</button></a>");
    client.println("</body></html>");
    
    client.stop();
    Serial.println("Client disconnected");
  }
}
