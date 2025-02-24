/* MQTT-HASS library by Andrew Maier
 */

#include "MQTT_HASS.h"

/**
 * Constructor.
 */
MQTT_HASS::MQTT_HASS(const char *domain, uint16_t port) {
}

MQTT_HASS::MQTT_HASS(const uint8_t *ip, uint16_t port) {
}

MQTT_HASS::~MQTT_HASS(){
}

bool MQTT_HASS::connect(const char *id) {
    return false;
}

bool MQTT_HASS::connect(const char *id, const char *user, const char *pass) {
    return false;
}

bool MQTT_HASS::isConnected() {
    return false;
}

void MQTT_HASS::disconnect(){
}

void MQTT_HASS::clear() {
}

bool MQTT_HASS::loop() {
    return false;
}

BinarySensor::BinarySensor(const String name, const String displayName, DeviceClasses deviceClasses, MQTT_HASS &client, Device dev)
: Entity(client, dev, name, displayName) 
, deviceClass_(deviceClasses)
{
  Entity::init("homeassistant/binary_sensor/particle_" + dev.name + "/" + name + "/");
}

bool BinarySensor::publishDiscovery() {
  char payload[2048];
  memset(payload, 0, sizeof(payload));
  JSONBufferWriter writer(payload, sizeof(payload));

  writer.beginObject();
  writer.name("name").value(Entity::displayName_);
  writer.name("state_topic").value(Entity::topicBase_ + "state");
  writer.name("availability_topic").value(Entity::topicBase_ + "availability");
  writer.name("unique_id").value(dev_.uniqueId + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  writer.name("device_class").value(deviceClasses2Str[deviceClass_]);
  writer.endObject();

  return Entity::publishDiscovery(payload);
}

bool BinarySensor::publishAvailability() {
  return Entity::publishAvailability();
}

bool BinarySensor::updateValue(Values val) {
  return Entity::publishState(values2Str[val]);
}

const char* BinarySensor::values2Str[BinarySensor::__VALUES_MAX] = {
  "OFF", // Values::OFF
  "ON",  // Values::ON
};

const char* BinarySensor::deviceClasses2Str[BinarySensor::__DEVICE_CLASSES_MAX] = {
  "None",
  "battery",
  "battery_charging",
  "carbon_monoxide",
  "cold",
  "connectivity",
  "door",
  "garage_door",
  "gas",
  "heat",
  "light",
  "lock",
  "moisture",
  "motion",
  "moving",
  "occupancy",
  "opening",
  "plug",
  "power",
  "presence",
  "problem",
  "running",
  "safety",
  "smoke",
  "sound",
  "tamper",
  "update",
  "vibration",
  "window",
};

void Entity::init(String topicBase, void (*callbackPtr)(char*, uint8_t*, unsigned int)) {
  topicBase_ = topicBase;
  callbackPtr_ = callbackPtr;
}

bool Entity::publishDiscovery(const char *configJSON) { return client_.publish(topicBase_ + "config", configJSON); }
bool Entity::publishAvailability() { return client_.publish(topicBase_ + "availability", "online"); }
bool Entity::publishState(String state) { return client_.publish(topicBase_ + "state", state); }

void Entity::fillDeviceJSON(JSONBufferWriter &writer) {
  writer.name("device").beginObject();
    writer.name("identifiers").beginArray();
        writer.name("particle_" + dev_.name);
    writer.endArray();
    writer.name("name").value(dev_.name);
    writer.name("manufacturer").value(dev_.manufacturer);
    writer.name("model").value(dev_.model);
    writer.name("sw_version").value(dev_.swVersion);
  writer.endObject();
}
