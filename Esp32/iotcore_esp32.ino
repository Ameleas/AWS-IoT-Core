
// Include required libraries for WiFi, secure connections, MQTT, and JSON parsing
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC "/telemetry"      // Topic for publishing sensor data
#define AWS_IOT_SUBSCRIBE_TOPIC "/downlink"     // Topic for receiving commands from AWS

#define DHTPIN 5
#define LDRPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

long sendInterval = 10000;  // Default interval (in milliseconds) at which to send telemetry to AWS

String THINGNAME = "";  // Will store the device's unique name (derived from MAC address)

// Initialize secure WiFi client and MQTT client with 1024 byte buffer for messages
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(1024);


// Establishes connection to AWS IoT Core via WiFi and MQTT
void connectAWS() {
  // Set WiFi to station mode (client mode, not access point)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Get the MAC address to use as unique device identifier
  THINGNAME = WiFi.macAddress();
  // Remove colons from the MAC address string (AWS IoT thing names cannot contain colons)
  for (int i = 0; i < THINGNAME.length(); i++) {
    if (THINGNAME.charAt(i) == ':') {
      THINGNAME.remove(i, 1);
      i--;
    }
  }

  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(THINGNAME);
  Serial.println("Connecting to Wi-Fi");

  // Wait until WiFi connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials for mutual TLS authentication
  net.setCACert(AWS_CERT_CA);           // Root CA certificate
  net.setCertificate(AWS_CERT_CRT);     // Device certificate
  net.setPrivateKey(AWS_CERT_PRIVATE);  // Device private key

  // Connect to the MQTT broker on the AWS endpoint (port 8883 is the standard MQTT over TLS port)
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Register callback function to handle incoming MQTT messages
  client.onMessage(messageHandler);

  Serial.println("Connecting to AWS IOT");

  // Attempt to connect to AWS IoT using the thing name as client ID
  while (!client.connect(THINGNAME.c_str())) {
    Serial.print(".");
    delay(100);
  }

  // Verify connection was successful
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to the downlink topic to receive commands from AWS
  client.subscribe(THINGNAME + AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}


// Configures AWS IoT Device Shadow subscriptions and requests current shadow state
void setupShadow() {
  // Subscribe to shadow topics to receive shadow updates
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/accepted");  // Receive accepted shadow get response
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/get/rejected");  // Receive rejected shadow get response
  //client.subscribe("$aws/things/" + THINGNAME + "/shadow/update/accepted");  // Optional: receive update confirmations
  client.subscribe("$aws/things/" + THINGNAME + "/shadow/update/delta");   // Receive delta updates (when desired state changes)

  // Request the current device shadow state from AWS
  client.publish("$aws/things/" + THINGNAME + "/shadow/get");
}


// Publishes telemetry data to AWS IoT Core
void publishTelemetry()
{
  //Create a JSON document of size 200 bytes, and populate it
  StaticJsonDocument<200> doc;

  float h = dht.readHumidity();  
  float t = dht.readTemperature();
  int lightValue = analogRead(LDRPIN);
  int l = (4095 - lightValue) * 100 / 4095; // convert to percentage

  doc["temperature"] = t;
  doc["humidity"] = h;
  doc["light"] = l;
  char jsonBuffer[128];
  serializeJson(doc, jsonBuffer); // print to mqtt_client

  //Publish to the topic
  bool sendResult = client.publish(THINGNAME + AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  if (sendResult == 1) {
    Serial.print("Sent message: ");
    Serial.println(jsonBuffer);
  } else {
    Serial.println("Send message failed");
  }
}

// Callback function that handles incoming MQTT messages
void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Parse the JSON payload
  JsonDocument doc;
  deserializeJson(doc, payload);

  // Handle device shadow messages
  if (topic.endsWith("/shadow/get/accepted")) {
    // Received accepted shadow get response - extract desired state
    updateSettings(doc["state"]["desired"]);
  } else if (topic.endsWith("/shadow/update/delta")) {
    // Received shadow delta update - extract the changed state
    updateSettings(doc["state"]);
  }
}

// Updates device settings based on shadow document and reports back to AWS
void updateSettings(JsonDocument settingsObj) {

  Serial.println("Received DeviceShadow document: ");
  serializeJson(settingsObj, Serial);
  Serial.println();

  // Update send interval if present in shadow document (convert seconds to milliseconds)
  if (settingsObj.containsKey("sendIntervalSeconds")) {
    sendInterval = settingsObj["sendIntervalSeconds"].as<int>() * 1000;
    Serial.println("Send interval updated to: " + String(sendInterval / 1000) + " seconds");
  }

  // Create response document to report current state back to shadow
  JsonDocument docResponse;
  docResponse["state"]["reported"] = settingsObj;  // Report the received settings as current state
  char jsonBuffer[512];
  serializeJson(docResponse, jsonBuffer);

  // Report back to device shadow to acknowledge the settings were applied
  Serial.print("Sending reported state to AWS: ");
  serializeJson(docResponse, Serial);
  Serial.println();

  client.publish("$aws/things/" + THINGNAME + "/shadow/update", jsonBuffer);
}

// Arduino setup function - runs once at startup
void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging
  delay(2000);            // Wait for serial monitor to connect
  connectAWS();           // Establish connection to AWS IoT Core
  setupShadow();          // Configure and request device shadow
  dht.begin();
}

// Arduino main loop - runs continuously
void loop() {
  // Use static variable to track last send time (initialized to allow immediate first send)
  static unsigned long previousMillis = -sendInterval;

  // Check if it's time to send telemetry data
  if (millis() - previousMillis >= sendInterval) {
    // Update the last send time
    previousMillis = millis();
    publishTelemetry();
  }

  // Process incoming MQTT messages and maintain connection
  client.loop();
}