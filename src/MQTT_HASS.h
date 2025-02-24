#pragma once

/* MQTT-HASS library by Andrew Maier
 */

// This will load the definition for common Particle variable types
#include <MQTT.h>

class Entity;

#define MQTT_PACKET_SIZE 2048

namespace Utils {
  String getSerialNum();
}

// This is your main class that users will import into their application
class MQTT_HASS : public MQTT {
public:
  /**
   * Constructors
   */
  MQTT_HASS() = delete;
  static MQTT_HASS& getInstance(const char* domain, uint16_t port) {
    static MQTT_HASS instance(domain, port);
    return instance;
  }

  static MQTT_HASS& getInstance(const uint8_t *ip, uint16_t port) {
    static MQTT_HASS instance(ip, port);
    return instance;
  }
  
  bool connect(const char *username, const char *password);
  bool registerEntity(Entity *entity);
  bool publishAvailabilities();

  void globalCallback(char* topic, uint8_t* payload, unsigned int length);

private:
  MQTT_HASS(const char *domain, uint16_t port);
  MQTT_HASS(const uint8_t *ip, uint16_t port);
  ~MQTT_HASS();
  Vector<Entity*> entities_;

  void init();

  static MQTT_HASS *instance_;
  static void globalCallbackWrapper(char* topic, uint8_t* payload, unsigned int length);
};

/**
 * @brief Struct representing a device.
 * 
 * This struct holds information about a device, including its name, model, and unique identifier.
 */
typedef struct {
  String name;                                /**< The name of the device. (DO NOT USE ANY SPACES) */
  String model;                               /**< The model of the device. */
  String swVersion = "1.0";                   /**< The software version of the device. */
  String manufacturer = "Particle MQTT_HASS"; /**< The manufacturer of the device. */
} Device;

class Entity {
public:
  String topicBase_;
  void (*callbackPtr_)(char*, uint8_t*, unsigned int);
  virtual bool publishDiscovery() = 0;
  bool publishAvailability();

protected:

  Entity() = delete;
  Entity(MQTT_HASS &client, Device dev, String name, String displayName)
  : client_(client)
  , dev_(dev)
  , name_(name)
  , displayName_(displayName)
  {}

  void init(String topicBase, void (*callbackPtr)(char*, uint8_t*, unsigned int) = nullptr);
  bool publishDiscovery(const char *config);
  bool publishState(String state);
  void fillDeviceJSON(JSONBufferWriter &writer);

  MQTT_HASS &client_;
  Device dev_;
  String name_;
  String displayName_;
};

class BinarySensor : public Entity {
public:  
  enum States {
    OFF,
    ON,
    __STATES_MAX,
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

  Entity *asEntity() { return this; }
  BinarySensor() = delete;
  BinarySensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClass = DeviceClasses::None);

  bool publishDiscovery();
  bool publishAvailability();
  bool updateState(States val);

private:
  DeviceClasses deviceClass_;

  const char* states2Str[__STATES_MAX] = {
    "OFF", // States::OFF
    "ON",  // States::ON
  };
  
  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
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
};

class Sensor : public Entity {
public:
  enum DeviceClasses {
    None,
    apparent_power,
    aqi,
    area,
    atompsheric_pressure,
    battery,
    blood_glucose_concentration,
    carbon_dioxide,
    carbon_monoxide,
    current,
    data_rate,
    data_size,
    date,
    distance,
    duration,
    energy,
    energy_storage,
    enumType,
    frequency,
    gas,
    humidity,
    illuminance,
    irradiance,
    moisture,
    monetary,
    nitrogen_dioxide,
    nitrogen_monoxide,
    nitrous_oxide,
    ozone,
    ph,
    pm1,
    pm25,
    pm10,
    power_factor,
    power,
    precipitation,
    precipitation_intensity,
    pressure,
    reactive_power,
    signal_strength,
    sound_pressure,
    speed,
    sulphur_dioxide,
    temperature,
    timestamp,
    volatile_organic_compounds,
    volatile_organic_compounds_parts,
    voltage,
    volume,
    volume_flow_rate,
    volume_storage,
    water,
    weight,
    wind_speed,
    __DEVICE_CLASSES_MAX,
  };

  Sensor() = delete;
  Sensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClass = DeviceClasses::None,
         String unitOfMeasurement = "");

  bool publishDiscovery();
  bool publishAvailability();
  bool updateState(String val);

private:
  DeviceClasses deviceClass_;
  String unitOfMeasurement_;

  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
    "None",                           // DeviceClasses::None
    "apparent_power",                 // DeviceClasses::apparent_power
    "aqi",                            // DeviceClasses::aqi
    "area",                           // DeviceClasses::area
    "atompsheric_pressure",          // DeviceClasses::atompsheric_pressure
    "battery",                       // DeviceClasses::battery
    "blood_glucose_concentration",  // DeviceClasses::blood_glucose_concentration
    "carbon_dioxide",              // DeviceClasses::carbon_dioxide
    "carbon_monoxide",          // DeviceClasses::carbon_monoxide
    "current",                // DeviceClasses::current
    "data_rate",            // DeviceClasses::data_rate
    "data_size",          // DeviceClasses::data_size
    "date",             // DeviceClasses::date
    "distance",       // DeviceClasses::distance
    "duration",     // DeviceClasses::duration 
    "energy",    // DeviceClasses::energy
    "energy_storage", // DeviceClasses::energy_storage
    "enum",         // DeviceClasses::enumType
    "frequency",  // DeviceClasses::frequency
    "gas",      // DeviceClasses::gas
    "humidity", // DeviceClasses::humidity
    "illuminance", // DeviceClasses::illuminance
    "irradiance", // DeviceClasses::irradiance
    "moisture", // DeviceClasses::moisture
    "monetary", // DeviceClasses::monetary
    "nitrogen_dioxide", // DeviceClasses::nitrogen_dioxide
    "nitrogen_monoxide", // DeviceClasses::nitrogen_monoxide
    "nitrous_oxide", // DeviceClasses::nitrous_oxide
    "ozone", // DeviceClasses::ozone
    "ph", // DeviceClasses::ph
    "pm1", // DeviceClasses::pm1
    "pm25",  // DeviceClasses::pm25
    "pm10", // DeviceClasses::pm10
    "power_factor", // DeviceClasses::power_factor
    "power", // DeviceClasses::power
    "precipitation", // DeviceClasses::precipitation
    "precipitation_intensity", // DeviceClasses::precipitation_intensity
    "pressure", // DeviceClasses::pressure
    "reactive_power", // DeviceClasses::reactive_power
    "signal_strength", // DeviceClasses::signal_strength
    "sound_pressure", // DeviceClasses::sound_pressure
    "speed", // DeviceClasses::speed
    "sulphur_dioxide", // DeviceClasses::sulphur_dioxide
    "temperature", // DeviceClasses::temperature
    "timestamp", // DeviceClasses::timestamp
    "volatile_organic_compounds", // DeviceClasses::volatile_organic_compounds
    "volatile_organic_compounds_parts", // DeviceClasses::volatile_organic_compounds_parts
    "voltage", // DeviceClasses::voltage
    "volume", // DeviceClasses::volume
    "volume_flow_rate",
    "volume_storage",
    "water",
    "weight",
    "wind_speed",
  };

};

class Button : public Entity {
public:
  enum DeviceClasses {
    None,
    identify,
    restart,
    update,
    __DEVICE_CLASSES_MAX,
  };

  Button() = delete;
  Button(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int),
         DeviceClasses deviceClass = DeviceClasses::None);

  bool publishDiscovery();
  bool publishAvailability();

private:
  DeviceClasses deviceClass_;

  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
    "None",     // DeviceClasses::None
    "identify", // DeviceClasses::identify
    "restart",  // DeviceClasses::restart
    "update",   // DeviceClasses::update
  };
};

class Lock : public Entity {
public:
  enum States {
    UNLOCKED,
    UNLOCKING,
    LOCKED,
    LOCKING,
    JAMMED,
    __STATES_MAX,
  };

  Lock() = delete;
  Lock(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int));

  bool publishDiscovery();
  bool publishAvailability();
  bool updateState(States val);
private:
  const char* states2Str[__STATES_MAX] = {
    "UNLOCKED", // States::UNLOCKED
    "UNLOCKING",// States::UNLOCKING
    "LOCKED",   // States::LOCKED
    "LOCKING",  // States::LOCKING
    "JAMMED",   // States::JAMMED
  };
};

class Cover : public Entity {
public:
  enum States {
    OPEN,
    CLOSED,
    OPENING,
    CLOSING,
    STOPPED,
    __STATES_MAX,
  };

  enum DeviceClasses {
    None,
    awning,
    blind,
    curtain,
    damper,
    door,
    garage,
    gate,
    shutter,
    window,
    __DEVICE_CLASSES_MAX,
  };

  Cover() = delete;
  Cover(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int),
        DeviceClasses deviceClass = DeviceClasses::None);

  bool publishDiscovery();
  bool publishAvailability();
  bool updateState(States val);

private:
  DeviceClasses deviceClass_;
  const char* states2Str[__STATES_MAX] = {
    "open",   // States::OPEN
    "closed", // States::CLOSED
    "opening",// States::OPENING
    "closing",// States::CLOSING
    "stopped",// States::STOPPED
  };

  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
    "None",   // DeviceClasses::None
    "awning", // DeviceClasses::awning
    "blind",  // DeviceClasses::blind
    "curtain",// DeviceClasses::curtain
    "damper", // DeviceClasses::damper
    "door",   // DeviceClasses::door
    "garage", // DeviceClasses::garage
    "gate",   // DeviceClasses::gate
    "shutter",// DeviceClasses::shutter
    "window", // DeviceClasses::window
  };
};

