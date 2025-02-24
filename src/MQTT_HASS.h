#pragma once

/* MQTT-HASS library by Andrew Maier
 */

// This will load the definition for common Particle variable types
#include <MQTT.h>

// This is your main class that users will import into their application
class MQTT_HASS : public MQTT {
public:
  /**
   * Constructors
   */
  MQTT_HASS() = delete;
  MQTT_HASS(const char* domain, uint16_t port);
  MQTT_HASS(const uint8_t *ip, uint16_t port);
  ~MQTT_HASS();

  bool connect(const char *id);
  bool connect(const char *id, const char *user, const char *pass);
  bool isConnected();
  void disconnect();

  void clear();
  bool loop();

private:
  char serial_num[HAL_DEVICE_SERIAL_NUMBER_SIZE + 1];
};

/**
 * @brief Struct representing a device.
 * 
 * This struct holds information about a device, including its name, model, and unique identifier.
 */
typedef struct {
  String name;    /**< The name of the device. (DO NOT USE ANY SPACES) */
  String model;   /**< The model of the device. */
  String uniqueId;/**< The unique identifier of the device. (i.e. the device serial number) */
  String swVersion = "1.0"; /**< The software version of the device. */
  String manufacturer = "Particle MQTT_HASS"; /**< The manufacturer of the device. */
} Device;

class Entity {
  protected:
  void (*callbackPtr_)(char*, uint8_t*, unsigned int);

  Entity() = delete;
  Entity(MQTT_HASS &client, Device dev, String name, String displayName)
  : client_(client)
  , dev_(dev)
  , name_(name)
  , displayName_(displayName)
  {}

  void init(String topicBase, void (*callbackPtr)(char*, uint8_t*, unsigned int) = nullptr);

  bool publishDiscovery(const char *config);
  bool publishAvailability();
  bool publishState(String state);
  void fillDeviceJSON(JSONBufferWriter &writer);

protected:
  MQTT_HASS &client_;
  Device dev_;
  String name_;
  String displayName_;
  String topicBase_;
};

class BinarySensor : protected Entity {
public:  
  enum Values {
    OFF,
    ON,
    __VALUES_MAX,
  };

  enum DeviceClasses {
    None,
    battery,
    battery_charging,
    carbon_monoxide,
    cold,
    connectivity,
    door,
    garage_door,
    gas,
    heat,
    light,
    lock,
    moisture,
    motion,
    moving,
    occupancy,
    opening,
    plug,
    power,
    presence,
    problem,
    running,
    safety,
    smoke,
    sound,
    tamper,
    update,
    vibration,
    window,
    __DEVICE_CLASSES_MAX,
  };

  BinarySensor() = delete;
  BinarySensor(const String name, const String displayName, DeviceClasses deviceClass, MQTT_HASS &client, Device dev);

  bool publishDiscovery();
  bool publishAvailability();
  bool updateValue(Values val);

private:
  DeviceClasses deviceClass_;

  static const char* values2Str[__VALUES_MAX];
  static const char* deviceClasses2Str[__DEVICE_CLASSES_MAX];
};

class Sensor : protected Entity {
public:
enum Types {
  None,
  apparent_power,
  aqi,

};
  // Sensor();
};

