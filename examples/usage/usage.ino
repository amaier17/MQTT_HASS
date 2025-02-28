// Example usage for MQTT_HASS library by Andrew Maier.

#include "MQTT_HASS.h"

// Get the MQTT_HASS static instance
byte mqtt_server[] = {192, 168, 0, 3};
MQTT_HASS& client = MQTT_HASS::getInstance(mqtt_server, 1883);

// Create a device object
Device dev = {
    .name = "imatesterbaby",
    .model = "Particle Argon",
};

// Define our callbacks
void buttonCallback(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Button pressed");
}

void buttonCallback2(char* topic, uint8_t* payload, unsigned int length) {
    Serial.println("Button 2 pressed");
}

void garagecallback(char* topic, uint8_t* payload, unsigned int length) {
    char p[length +1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);

    Serial.println("Garage door " + message);
}

// Create our sensors
BinarySensor tamper("tamper", "Tamper Sensor", client, dev, BinarySensor::DeviceClasses::tamper);
Sensor temperature("temperature", "Temperature Sensor", client, dev, Sensor::DeviceClasses::temperature, String("\xb0") + String("C"));
Sensor garageHealth("garageHealth", "Garage Door Diagnostic Health Sensor", client, dev, Sensor::DeviceClasses::None, "", Sensor::EntityCategories::diagnostic);
Button myButton("button", "Button but bigger", client, dev, buttonCallback);
Button myButton2("button2", "Button but bigger2", client, dev, buttonCallback2);
Cover myGarage("garage", "Garage Door", client, dev, garagecallback, Cover::DeviceClasses::garage);

// Initialize objects from the lib
void setup() {
    waitUntil(Particle.connected);
    Serial.begin();
    Serial.println("Trying connect");
    client.connect("mqtt_user", "mqtt_password");
    if (client.isConnected()) {
        Serial.println("Connected");
        client.registerEntity(&tamper);
        client.registerEntity(&temperature);
        client.registerEntity(&garageHealth);
        client.registerEntity(&myButton);
        client.registerEntity(&myButton2);
        client.registerEntity(&myGarage);
    } else {
        Serial.println("Not connected");
    }
}

void loop() {
    static int i = 0;
    if (client.isConnected()) {
        if (i % 10 == 0) {
            tamper.updateState(BinarySensor::States::ON);
            temperature.updateState(String(25 + i));
            garageHealth.updateState("healthy");
            Serial.println("Changing temperature to " + String(25 + i));
        } else {
            tamper.updateState(BinarySensor::States::OFF);
        }
        i++;
        delay(1000);
        client.publishAvailabilities();
        client.loop();
    } else {
        client.connect("mqtt_user", "mqtt_password");
        if (client.isConnected()) {
            Serial.println("Connected");
            client.registerEntity(&tamper);
            client.registerEntity(&myButton);
            client.registerEntity(&myButton2);
            client.registerEntity(&myGarage);
            client.registerEntity(&temperature);
        }
    }
}
