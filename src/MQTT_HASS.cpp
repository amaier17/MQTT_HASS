/* MQTT-HASS library by Andrew Maier
 */

#include "MQTT_HASS.h"

MQTT_HASS *MQTT_HASS::instance_ = nullptr;

MQTT_HASS::MQTT_HASS(const char *domain, uint16_t port)
: MQTT(domain, port, MQTT_PACKET_SIZE, globalCallbackWrapper) {
  init();
}


MQTT_HASS::MQTT_HASS(const uint8_t *ip, uint16_t port)
: MQTT(ip, port, MQTT_PACKET_SIZE, globalCallbackWrapper) {
  init();
}

MQTT_HASS::~MQTT_HASS() {
}


bool MQTT_HASS::connect(const char *username, const char *password) {
	if (MQTT::isConnected())
		return true;

  if (!entities_.isEmpty())
    entities_.clear();
  if (!MQTT::connect("particle" + Utils::getSerialNum() + String(Time.now()),username, password))
		return false;

	return !MQTT::subscribe("homeassistant/status");
}

bool MQTT_HASS::registerEntity(Entity *entity)
{
    entity->publishDiscovery();
    return entities_.append(entity);
}

bool MQTT_HASS::publishAvailabilities() {
  for (auto it = entities_.begin(); it != entities_.end(); it++) {
    Entity *entity = *it;
    if (!entity->publishAvailability())
      return false;
  }

  return true;
}

void MQTT_HASS::globalCallback(char *topic, uint8_t *payload, unsigned int length) {
  String topic_str(topic);
	char p[length + 1];
	memcpy(p, payload, length);
	p[length] = NULL;
	String message(p);
	
	// If we're given the "birth" message we need to resend the config
	if (topic_str == "homeassistant/status") {
		if (message == "online") {
			for (auto it = entities_.begin(); it != entities_.end(); it++) {
				Entity *entity = *it;
				entity->publishDiscovery();
			}
			publishAvailabilities();
		}
	} else {
		for (auto it = entities_.begin(); it != entities_.end(); it++) {
			Entity *entity = *it;
			if (topic_str == entity->topicBase_ + "command" && entity->callbackPtr_ != nullptr)
			entity->callbackPtr_(topic, payload, length);
		}
	}
}

void MQTT_HASS::init() {
  instance_ = this;
}

void MQTT_HASS::globalCallbackWrapper(char *topic, uint8_t *payload, unsigned int length)
{
    if (instance_)
        instance_->globalCallback(topic, payload, length);
}

BinarySensor::BinarySensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClasses)
: Entity(client, dev, name, displayName) 
, deviceClass_(deviceClasses) {
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
  writer.name("unique_id").value(Utils::getSerialNum() + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  if (deviceClass_ != DeviceClasses::None)
    writer.name("device_class").value(deviceClasses2Str[deviceClass_]);
  writer.endObject();

  return Entity::publishDiscovery(payload);
}

bool BinarySensor::publishAvailability() { return Entity::publishAvailability(); }
bool BinarySensor::updateState(States val) { return Entity::publishState(states2Str[val]); }


void Entity::init(String topicBase, void (*callbackPtr)(char*, uint8_t*, unsigned int)) {
  topicBase_ = topicBase;
  callbackPtr_ = callbackPtr;
}

bool Entity::publishDiscovery(const char *configJSON)
{
    if (!client_.publish(topicBase_ + "config", configJSON))
        return false;

    if (callbackPtr_ != nullptr)
    {
        if (!client_.subscribe(topicBase_ + "command"))
            return false;
    }

    return true;
}

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

Sensor::Sensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClass,
               String unitOfMeasurement, EntityCategories entityCategory)
: Entity(client, dev, name, displayName)
, deviceClass_(deviceClass)
, unitOfMeasurement_(unitOfMeasurement)
, entityCategory_(entityCategory) {
    Entity::init("homeassistant/sensor/particle_" + dev.name + "/" + name + "/");
}

bool Sensor::publishDiscovery() {
  char payload[2048];
  memset(payload, 0, sizeof(payload));
  JSONBufferWriter writer(payload, sizeof(payload));

  writer.beginObject();
  writer.name("name").value(Entity::displayName_);
  writer.name("state_topic").value(Entity::topicBase_ + "state");
  writer.name("availability_topic").value(Entity::topicBase_ + "availability");
  writer.name("unique_id").value(Utils::getSerialNum() + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  if (deviceClass_ != DeviceClasses::None)
    writer.name("device_class").value(deviceClasses2Str[deviceClass_]);
  if (unitOfMeasurement_ != "")
    writer.name("unit_of_measurement").value(unitOfMeasurement_);
  if (entityCategory_ == EntityCategories::diagnostic)
    writer.name("entity_category").value("diagnostic");
  writer.endObject();

  return Entity::publishDiscovery(payload);
}

bool Sensor::publishAvailability() { return Entity::publishAvailability(); }
bool Sensor::updateState(String val) {
  Serial.println("I'm updating val to " + val);
  return Entity::publishState(val); }

Button::Button(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int), DeviceClasses deviceClass)
: Entity(client, dev, name, displayName)
, deviceClass_(deviceClass) {
    Entity::init("homeassistant/button/particle_" + dev.name + "/" + name + "/", callbackPtr);
}

bool Button::publishDiscovery() {
  char payload[2048];
  memset(payload, 0, sizeof(payload));
  JSONBufferWriter writer(payload, sizeof(payload));

  writer.beginObject();
  writer.name("name").value(Entity::displayName_);
  writer.name("command_topic").value(Entity::topicBase_ + "command");
  writer.name("availability_topic").value(Entity::topicBase_ + "availability");
  writer.name("unique_id").value(Utils::getSerialNum() + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  if (deviceClass_ != DeviceClasses::None)
    writer.name("device_class").value(deviceClasses2Str[deviceClass_]);
  writer.endObject();

  return Entity::publishDiscovery(payload);
}

bool Button::publishAvailability() { return Entity::publishAvailability(); }

Lock::Lock(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char *, uint8_t *, unsigned int))
: Entity(client, dev, name, displayName) {
    Entity::init("homeassistant/lock/particle_" + dev.name + "/" + name + "/", callbackPtr);
}

bool Lock::publishDiscovery() {
  char payload[2048];
  memset(payload, 0, sizeof(payload));
  JSONBufferWriter writer(payload, sizeof(payload));

  writer.beginObject();
  writer.name("name").value(Entity::displayName_);
  writer.name("state_topic").value(Entity::topicBase_ + "state");
  writer.name("command_topic").value(Entity::topicBase_ + "command");
  writer.name("availability_topic").value(Entity::topicBase_ + "availability");
  writer.name("unique_id").value(Utils::getSerialNum() + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  writer.endObject();

  return Entity::publishDiscovery(payload);
}
bool Lock::publishAvailability() { return Entity::publishAvailability(); }
bool Lock::updateState(States val) { return Entity::publishState(states2Str[val]); }

Cover::Cover(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char *, uint8_t *, unsigned int),
             DeviceClasses deviceClass)
: Entity(client, dev, name, displayName)
, deviceClass_(deviceClass) {
    Entity::init("homeassistant/cover/particle_" + dev.name + "/" + name + "/", callbackPtr);
}

bool Cover::publishDiscovery() {
  char payload[2048];
  memset(payload, 0, sizeof(payload));
  JSONBufferWriter writer(payload, sizeof(payload));

  writer.beginObject();
  writer.name("name").value(Entity::displayName_);
  writer.name("state_topic").value(Entity::topicBase_ + "state");
  writer.name("command_topic").value(Entity::topicBase_ + "command");
  writer.name("availability_topic").value(Entity::topicBase_ + "availability");
  writer.name("unique_id").value(Utils::getSerialNum() + "_" + Entity::name_);
  Entity::fillDeviceJSON(writer);
  if (deviceClass_ != DeviceClasses::None)
    writer.name("device_class").value(deviceClasses2Str[deviceClass_]);
  writer.endObject();

  return Entity::publishDiscovery(payload);
}

bool Cover::publishAvailability() { return Entity::publishAvailability(); }
bool Cover::updateState(States val) { return Entity::publishState(states2Str[val]); }

String Utils::getSerialNum()
{
  char serialNum[HAL_DEVICE_SERIAL_NUMBER_SIZE + 1];
  memset(serialNum, 0, sizeof(serialNum));
  hal_get_device_serial_number(serialNum, HAL_DEVICE_SERIAL_NUMBER_SIZE, nullptr);

  return String(serialNum);
}
