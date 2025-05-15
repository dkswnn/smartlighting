#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Dkswn";
const char* password = "123456789";
const char* mqtt_server = "192.168.137.1";
const int mqtt_port = 1883;
const char* mqtt_topic = "school/lighting/control";

const int ledPin = 33;
int brightness = 255;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    ledcSetup(0, 10000, 8);
    ledcAttachPin(ledPin, 0);
    ledcWrite(0, brightness);
    Serial.println("Smart Lighting with Dimming Started");

    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
        switch (WiFi.status()) {
            case WL_NO_SSID_AVAIL:
                Serial.println("SSID not found!");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("Connection failed - wrong password?");
                break;
            case WL_DISCONNECTED:
                Serial.println("Disconnected - still trying...");
                break;
            default:
                break;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC address: ");
        Serial.println(WiFi.macAddress());
    } else {
        Serial.println("");
        Serial.println("Failed to connect to WiFi after 20 attempts!");
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("Message received on topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    if (message.indexOf("state") != -1) {
        if (message.indexOf("\"state\":\"ON\"") != -1) {
            int brightnessIndex = message.indexOf("\"brightness\":");
            if (brightnessIndex != -1) {
                int startIndex = brightnessIndex + 13;
                int endIndex = message.indexOf("}", startIndex);
                String brightnessStr = message.substring(startIndex, endIndex);
                int newBrightness = brightnessStr.toInt();
                brightness = map(newBrightness, 0, 100, 255, 0);
                ledcWrite(0, brightness);
                Serial.print("Received brightness: ");
                Serial.print(newBrightness);
                Serial.print("%, PWM value: ");
                Serial.println(brightness);
            }
        } else if (message.indexOf("\"state\":\"OFF\"") != -1) {
            brightness = 255;
            ledcWrite(0, brightness);
            Serial.println("LED turned OFF, PWM value: 255");
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.subscribe(mqtt_topic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    delay(50);
}