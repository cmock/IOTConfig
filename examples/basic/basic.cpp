#include <WiFi.h>
#include <PubSubClient.h>
#include <IOTConfig.h>

// change these:
#define WIFI_SSID "mySSID"
#define WIFI_PSK "TopSecret"
#define MQTT_SERVER "mqtt.example.com"
#define MQTT_PORT 1883

WiFiClient espClient;
PubSubClient mqttClient(espClient);

IOTConfig config; // The IOTConfig holder object

int myInt; // an IOTConfig managed variable

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("WiFi connected.");

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.connect("my-client-id");

  // init the holder; the bool decides whether the published values
  // are retained.
  config.begin(mqttClient, "myPrefsNamespace", "base/topic", true);

  // myInt will be read from Preferences (first bool),
  // writeable via MQTT at base/topic/config/myInt (second bool),
  // and defaults to 42.
  config.addVar(&myInt, "myInt", true, true, 42);
}

void loop() {
  myInt++; // use the variable as normal

  mqttClient.loop();

  // when variables have changed, publish them to MQTT (base/topic/myInt)
  // and store in Preferences.
  // Updating variables from MQTT is happening in a callback, no action needed.
  config.update();
  delay(10000);
}
