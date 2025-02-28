/**
 * @file MQTT_HASS.h
 * @brief Header file for managing MQTT integration with Home Assistant.
 * @author Andrew Maier
 * @date February 27, 2025
 *
 * This library defines a simpler interface for integrating Particle devices with Home Assistant
 * by using the existing MQTT library. It provides a set of classes to represent various Home
 * Assistant entities (e.g., binary sensors, sensors, buttons, locks, and covers). The entities support
 * automatic discovery, state publishing, and availability updates via MQTT.
 *
 * Key Classes and Structures:
 * ---------------------------------------------------------------------------
 * 1. MQTT_HASS
 *    - Singleton class derived from MQTT that handles connection management,
 *      global message callbacks, and integrates with Home Assistant through MQTT.
 *    - Provides two overloaded getInstance() methods accepting either a domain or an IP.
 *    - Exposes methods for connecting to the broker, registering entities, and publishing
 *      availability.
 *
 * 2. Device
 *    - A struct representing a device with essential information such as name, model,
 *      software version, and manufacturer.
 *
 * 3. BinarySensor
 *    - A subclass of Entity that represents a binary sensor (e.g., on/off).
 *    - Enumerates sensor states (OFF, ON) and a set of device classes for further classification.
 *    - Implements methods for publishing discovery messages and updating sensor state.
 *
 * 4. Sensor
 *    - A subclass of Entity that represents a sensor providing continuous or numerical
 *      measurement values.
 *    - Supports various device classes (e.g., temperature, humidity) and units of measurement.
 *    - Implements methods to publish discovery information, availability, and update its state.
 *
 * 5. Button
 *    - A subclass of Entity representing an actionable button (e.g., identify, restart).
 *    - Accepts a callback pointer to handle button actions via incoming MQTT messages.
 *    - Implements methods to publish discovery and availability messages.
 *
 * 6. Lock
 *    - A subclass of Entity representing a lock with states such as UNLOCKED, UNLOCKING,
 *      LOCKED, LOCKING, and JAMMED.
 *    - Provides a method to publish discovery data, availability, and update the current state.
 *
 * 7. Cover
 *    - A subclass of Entity representing a cover (e.g., blinds, curtains, garage doors).
 *    - Enumerates different states (open, closed, opening, closing, stopped) and device classes
 *      for various cover types.
 *    - Implements methods to publish discovery data, availability, and update its state.
 *
 * Overall, the design facilitates the integration of diverse Home Assistant entities with
 * MQTT by providing a unified discovery and control mechanism.
 */
#pragma once

#include <MQTT.h>

class Entity;
#define MQTT_PACKET_SIZE 2048

namespace Utils {
  String getSerialNum();
}


/**
 * @class MQTT_HASS
 * @brief Singleton class that extends MQTT for Home Assistant integration.
 *
 * MQTT_HASS is designed as a singleton to manage a single connection instance to an MQTT broker.
 * It provides additional functionality tailored for Home Assistant by allowing:
 *   - Connection to the MQTT broker using either a domain name or an IP address.
 *   - Registration of entities (devices or sensors) for Home Assistant.
 *   - Publication of availability statuses for registered entities.
 *   - Handling of incoming MQTT messages via a global callback mechanism.
 *
 * Usage:
 *   - Obtain the instance via:
 *       MQTT_HASS& instance = MQTT_HASS::getInstance(domain, port);
 *     or
 *       MQTT_HASS& instance = MQTT_HASS::getInstance(ip, port);
 *
 * Methods:
 *   - connect: Establishes a connection using provided username and password.
 *   - registerEntity: Registers an entity to be managed by Home Assistant.
 *   - publishAvailabilities: Publishes availability messages for all registered entities.
 *
 * @note This design enforces a single point of MQTT communication, ensuring consistent state and behavior
 *       across the application.
 */
class MQTT_HASS : public MQTT {
public:
  MQTT_HASS() = delete;

  /**
   * @brief Retrieves the singleton instance of the MQTT_HASS class.
   *
   * This method ensures that only a single instance of MQTT_HASS is created.
   * The instance is initialized with the provided domain and port during the first call.
   * Subsequent calls will return the same instance, regardless of the parameter values.
   *
   * @param domain A C-string representing the MQTT domain. (e.g., "mqtt.example.com")
   * @param port The port number for the MQTT connection. (e.g., 1883)
   * @return MQTT_HASS& A reference to the singleton instance of MQTT_HASS.
   */
  static MQTT_HASS& getInstance(const char* domain, uint16_t port) {
    static MQTT_HASS instance(domain, port);
    return instance;
  }

  /**
   * @brief Retrieves the singleton instance of the MQTT_HASS class.
   *
   * This method ensures that only a single instance of MQTT_HASS is created.
   * The instance is initialized with the provided domain and port during the first call.
   * Subsequent calls will return the same instance, regardless of the parameter values.
   *
   * @param ip An array of bytes representing the MQTT IP address. (e.g. {192, 168, 1, 1})
   * @param port The port number for the MQTT connection. (e.g., 1883)
   * @return MQTT_HASS& A reference to the singleton instance of MQTT_HASS.
   */
  static MQTT_HASS& getInstance(const uint8_t *ip, uint16_t port) {
    static MQTT_HASS instance(ip, port);
    return instance;
  }

  /**
   * @brief Connects to the server using a username and password.
   *
   * This function attempts to establish a connection to a service with the
   * provided authentication credentials.
   *
   * @param username A pointer to a null-terminated string representing the username.
   * @param password A pointer to a null-terminated string representing the password.
   * @return true if the connection is successfully established, false otherwise.
   */
  bool connect(const char *username, const char *password);

  /**
   * @brief Registers an entity to be managed by Home Assistant.
   *
   * This function adds an entity to the list of managed entities by the MQTT_HASS instance.
   * The entity will be responsible for publishing discovery data and state updates.
   *
   * @param entity A pointer to the entity object to be registered. (e.g. BinarySensor, Sensor, Button, etc)
   * @return true if the entity is successfully registered, false otherwise.
   */
  bool registerEntity(Entity *entity);

  /**
   * @brief Publishes availability messages for all registered entities.
   *
   * This function iterates over all registered entities and publishes availability messages
   * to indicate the current state of each entity.
	 *
	 * This function should be called on a regular basis (typically at least every 30 seconds)
	 *
	 * @note This function is typically called after registering all entities to ensure availability messages are sent.
   *
   * @return true if all availability messages are successfully published, false otherwise.
   */
  bool publishAvailabilities();

	/**
	 * @private
	 */
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

/**
 * @private
 */
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

/**
 * @class BinarySensor
 * @brief Represents a binary sensor entity in Home Assistant.
 *
 * BinarySensor is a subclass of Entity that models a binary sensor in Home Assistant.
 * It supports two states (ON and OFF) and a set of device classes for further classification.
 * The class provides methods for publishing discovery data, availability, and updating the sensor state.
 *
 * Usage:
 *  - Create an instance of BinarySensor with a name, display name, MQTT_HASS instance, and device information.
 *  - Register the sensor with the MQTT_HASS instance using registerEntity().
 *  - Update the sensor state using updateState() with the desired state (ON or OFF).
 *
 * @note This class simplifies the integration of binary sensors with Home Assistant by providing
 *       a unified interface for discovery, state updates, and availability management.
 */
class BinarySensor : public Entity {
public:
  /**
   * @brief Enumerates the possible states of a binary sensor.
   */
  enum States {
    OFF,          /**< Binary Sensor Off */
    ON,           /**< Binary Sensor On */
    __STATES_MAX,
  };

  /**
   * @brief Enumerates the possible device classes for a binary sensor.
   */
  enum DeviceClasses {
    None,             /**< No device class */
    battery,          /**< Battery sensor */
    battery_charging, /**< Battery charging sensor */
    carbon_monoxide,  /**< Carbon monoxide sensor */
    cold,             /**< Cold sensor */
    connectivity,     /**< Connectivity sensor */
    door,             /**< Door sensor */
    garage_door,      /**< Garage door sensor */
    gas,              /**< Gas sensor */
    heat,             /**< Heat sensor */
    light,            /**< Light sensor */
    lock,             /**< Lock sensor */
    moisture,         /**< Moisture sensor */
    motion,           /**< Motion sensor */
    moving,           /**< Moving sensor */
    occupancy,        /**< Occupancy sensor */
    opening,          /**< Opening sensor */
    plug,             /**< Plug sensor */
    power,            /**< Power sensor */
    presence,         /**< Presence sensor */
    problem,          /**< Problem sensor */
    running,          /**< Running sensor */
    safety,           /**< Safety sensor */
    smoke,            /**< Smoke sensor */
    sound,            /**< Sound sensor */
    tamper,           /**< Tamper sensor */
    update,           /**< Update sensor */
    vibration,        /**< Vibration sensor */
    window,           /**< Window sensor */
    __DEVICE_CLASSES_MAX,
  };

  BinarySensor() = delete;

  /**
   * @brief Constructs a BinarySensor object with the given parameters.
   *
   * This constructor initializes a BinarySensor object with the provided name, display name,
   * MQTT_HASS instance, device information, and optional device class.
   *
   * @param name The name of the binary sensor. (DO NOT USE ANY SPACES)
   * @param displayName The display name of the binary sensor.
   * @param client A reference to the MQTT_HASS instance.
   * @param dev The device information for the sensor.
   * @param deviceClass The device class for the sensor. (default: None)
   */
  BinarySensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClass = DeviceClasses::None);

  /**
   * @brief Publishes the discovery message for the binary sensor.
   *
   * This method generates the discovery message for the binary sensor and publishes it to the MQTT broker.
   * The message includes the sensor's name, state topic, availability topic, unique ID, device information,
   * and device class (if applicable).
   *
   * @note this is called automatically for you when you register the entity and does not need to be called manually.
   *
   * @return true if the discovery message is successfully published, false otherwise.
   */
  bool publishDiscovery();

  /**
   * @brief (Optionally) Publishes the availability message for the binary sensor.
   *
   * This method publishes an availability message for the binary sensor to indicate its current state.
   * The message is sent to the availability topic of the sensor.
   *
   * @note this is called automatically for you when you register the entity and call publishAvailabilities().
   *
   * @return true if the availability message is successfully published, false otherwise.
   */
  bool publishAvailability();

  /**
   * @brief Updates the state of the binary sensor.
   *
   * This method updates the state of the binary sensor to the specified value.
   * The state is published to the state topic of the sensor.
   *
   * @param val The new state of the binary sensor (BinarySensor::ON or BinarySensor::OFF).
   * @return true if the state is successfully updated, false otherwise.
   */
  bool updateState(States val);

private:
  DeviceClasses deviceClass_;

  const char* states2Str[__STATES_MAX] = {
    "OFF",
    "ON",
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

/**
 * @class Sensor
 * @brief Represents a sensor entity in Home Assistant.
 *
 * Sensor is a subclass of Entity that models a sensor in Home Assistant.
 * It provides continuous or numerical measurement values and supports various device classes
 * for further classification. The class also allows the specification of units of measurement.
 * Sensor implements methods for publishing discovery data, availability, and updating the sensor state.
 *
 * Usage:
 * - Create an instance of Sensor with a name, display name, MQTT_HASS instance, device information,
 *  device class, and unit of measurement.
 * - Register the sensor with the MQTT_HASS instance using registerEntity().
 * - Update the sensor state using updateState() with the desired value.
 *
 * @note This class simplifies the integration of sensors with Home Assistant by providing a unified
 *       interface for discovery, state updates, and availability management.
 */
class Sensor : public Entity {
public:

	/**
	 * @brief Enumerates the possible categories for a sensor entity.
	 */
  enum EntityCategories {
    normal,	    /**< normal */
    diagnostic, /**< diagnostic */
    __ENTITY_CATEGORIES_MAX,
  };

	/**
	 * @brief Enumerates the possible device classes for a sensor entity.
	 */
  enum DeviceClasses {
    None,                             /**< No device class */
    apparent_power,                   /**< Apparent power sensor */
    aqi,                              /**< Air quality index sensor */
    area,                             /**< Area sensor */
    atompsheric_pressure,             /**< Atmospheric pressure sensor */
    battery,                          /**< Battery sensor */
    blood_glucose_concentration,      /**< Blood glucose concentration sensor */
    carbon_dioxide,                   /**< Carbon dioxide sensor */
    carbon_monoxide,                  /**< Carbon monoxide sensor */
    current,                          /**< Current sensor */
    data_rate,                        /**< Data rate sensor */
    data_size,                        /**< Data size sensor */
    date,                             /**< Date sensor */
    distance,                         /**< Distance sensor */
    duration,                         /**< Duration sensor */
    energy,                           /**< Energy sensor */
    energy_storage,                   /**< Energy storage sensor */
    enumType,                         /**< Enum sensor */
    frequency,                        /**< Frequency sensor */
    gas,                              /**< Gas sensor */
    humidity,                         /**< Humidity sensor */
    illuminance,                      /**< Illuminance sensor */
    irradiance,                       /**< Irradiance sensor */
    moisture,                         /**< Moisture sensor */
    monetary,                         /**< Monetary sensor */
    nitrogen_dioxide,                 /**< Nitrogen dioxide sensor */
    nitrogen_monoxide,                /**< Nitrogen monoxide sensor */
    nitrous_oxide,                    /**< Nitrous oxide sensor */
    ozone,                            /**< Ozone sensor */
    ph,                               /**< PH sensor */
    pm1,                              /**< PM1 sensor */
    pm25,                             /**< PM2.5 sensor */
    pm10,                             /**< PM10 sensor */
    power_factor,                     /**< Power factor sensor */
    power,                            /**< Power sensor */
    precipitation,                    /**< Precipitation sensor */
    precipitation_intensity,          /**< Precipitation intensity sensor */
    pressure,                         /**< Pressure sensor */
    reactive_power,                   /**< Reactive power sensor */
    signal_strength,                  /**< Signal strength sensor */
    sound_pressure,                   /**< Sound pressure sensor */
    speed,                            /**< Speed sensor */
    sulphur_dioxide,                  /**< Sulphur dioxide sensor */
    temperature,                      /**< Temperature sensor */
    timestamp,                        /**< Timestamp sensor */
    volatile_organic_compounds,       /**< Volatile organic compounds sensor */
    volatile_organic_compounds_parts, /**< Volatile organic compounds parts sensor */
    voltage,                          /**< Voltage sensor */
    volume,                           /**< Volume sensor */
    volume_flow_rate,                 /**< Volume flow rate sensor */
    volume_storage,                   /**< Volume storage sensor */
    water,                            /**< Water sensor */
    weight,                           /**< Weight sensor */
    wind_speed,                       /**< Wind speed sensor */
    __DEVICE_CLASSES_MAX,
  };

  Sensor() = delete;

	/**
	 * @brief Constructs a Sensor object with the given parameters.
	 *
	 * This constructor initializes a Sensor object with the provided name, display name,
	 * MQTT_HASS instance, device information, device class, unit of measurement, and entity category.
	 *
	 * @param name The name of the sensor. (DO NOT USE ANY SPACES)
	 * @param displayName The display name of the sensor.
	 * @param client A reference to the MQTT_HASS instance.
	 * @param dev The device information for the sensor.
	 * @param deviceClass The device class for the sensor. (default none)
	 * @param unitOfMeasurement The unit of measurement for the sensor. (default "")
	 * @param entityCategory The entity category for the sensor. (default normal)
	 *
	 * @note The unit of measurement should be a string representing the unit (e.g., "°C" for Celsius).
	 *
	 * @return A Sensor object with the specified parameters.
	 */
  Sensor(const String name, const String displayName, MQTT_HASS &client, Device dev, DeviceClasses deviceClass = DeviceClasses::None,
         String unitOfMeasurement = "", EntityCategories entityCategory = EntityCategories::normal);

	/**
	 * @brief Publishes the discovery message for the sensor.
	 *
	 * @note this is called automatically for you when you register the entity and does not need to be called manually.
	 *
	 * @return true if the discovery message is successfully published, false otherwise.
	 */
  bool publishDiscovery();

	/**
	 * @brief Publishes the availability message for the sensor.
	 *
	 * @note this is called automatically for you when you register the entity and call publishAvailabilities().
	 *
	 * @return true if the availability message is successfully published, false otherwise.
	 */
  bool publishAvailability();

	/**
	 * @brief Updates the state of the sensor.
	 *
	 * This method updates the state of the sensor to the specified value.
	 * The state is published to the state topic of the sensor.
	 *
	 * @param val The new state of the sensor as a string.
	 * @return true if the state is successfully updated, false otherwise.
	 */
  bool updateState(String val);

private:
  DeviceClasses deviceClass_;
  String unitOfMeasurement_;
  EntityCategories entityCategory_;

  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
    "None",
    "apparent_power",
    "aqi",
    "area",
    "atompsheric_pressure",
    "battery",
    "blood_glucose_concentration",
    "carbon_dioxide",
    "carbon_monoxide",
    "current",
    "data_rate",
    "data_size",
    "date",
    "distance",
    "duration",
    "energy",
    "energy_storage",
    "enum",
    "frequency",
    "gas",
    "humidity",
    "illuminance",
    "irradiance",
    "moisture",
    "monetary",
    "nitrogen_dioxide",
    "nitrogen_monoxide",
    "nitrous_oxide",
    "ozone",
    "ph",
    "pm1",
    "pm25",
    "pm10",
    "power_factor",
    "power",
    "precipitation",
    "precipitation_intensity",
    "pressure",
    "reactive_power",
    "signal_strength",
    "sound_pressure",
    "speed",
    "sulphur_dioxide",
    "temperature",
    "timestamp",
    "volatile_organic_compounds",
    "volatile_organic_compounds_parts",
    "voltage",
    "volume",
    "volume_flow_rate",
    "volume_storage",
    "water",
    "weight",
    "wind_speed",
  };
};

/**
 * @class Button
 * @brief Represents a button entity in Home Assistant.
 *
 * Usage:
 * - Create an instance of Button with a name, display name, MQTT_HASS instance, device information, and callback pointer.
 * - Register the button with the MQTT_HASS instance using registerEntity().
 * - Handle button actions using the specified callback pointer.
 */
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

	/**
	 * @brief Constructs a Button object with the given parameters.
	 *
	 * This constructor initializes a Button object with the provided name, display name,
	 * MQTT_HASS instance, device information, and callback pointer.
	 *
	 * @param name The name of the button. (DO NOT USE ANY SPACES)
	 * @param displayName The display name of the button.
	 * @param client A reference to the MQTT_HASS instance.
	 * @param dev The device information for the button.
	 * @param callbackPtr A pointer to the callback function for handling button actions.
	 * @param deviceClass The device class for the button. (default None)
	 *
	 * @return A Button object with the specified parameters.
	 */
  Button(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int),
         DeviceClasses deviceClass = DeviceClasses::None);

	/**
	 * @brief Publishes the discovery message for the button.
	 *
	 * @note this is called automatically for you when you register the entity and does not need to be called manually.
	 *
	 * @return true if the discovery message is successfully published, false otherwise.
	 */
  bool publishDiscovery();

	/**
	 * @brief Publishes the availability message for the button.
	 *
	 * @note this is called automatically for you when you register the entity and call publishAvailabilities().
	 *
	 * @return true if the availability message is successfully published, false otherwise.
	 */
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

/**
 * @class Lock
 * @brief Represents a lock entity in Home Assistant.
 *
 * Usage:
 * - Create an instance of Lock with a name, display name, MQTT_HASS instance, device information, and callback pointer.
 * - Register the lock with the MQTT_HASS instance using registerEntity().
 * - Update the lock state using updateState() with the desired value.
 * - Handle lock actions using the specified callback pointer.
 *
 * @note This class simplifies the integration of locks with Home Assistant by providing a unified
 *  		 interface for discovery, state updates, and availability management.
 */
class Lock : public Entity {
public:

	/**
	 * @brief Enumerates the possible states of a lock entity.
	 */
  enum States {
    UNLOCKED,  /**< Unlocked */
    UNLOCKING, /**< Unlocking */
    LOCKED,    /**< Locked */
    LOCKING,   /**< Locking */
    JAMMED,    /**< Jammed */
    __STATES_MAX,
  };

  Lock() = delete;

	/**
	 * @brief Constructs a Lock object with the given parameters.
	 *
	 * This constructor initializes a Lock object with the provided name, display name,
	 * MQTT_HASS instance, device information, and callback pointer.
	 *
	 * @param name The name of the lock. (DO NOT USE ANY SPACES)
	 * @param displayName The display name of the lock.
	 * @param client A reference to the MQTT_HASS instance.
	 * @param dev The device information for the lock.
	 * @param callbackPtr A pointer to the callback function for handling lock actions.
	 *
	 * @return A Lock object with the specified parameters.
	 */
  Lock(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int));

	/**
	 * @brief Publishes the discovery message for the lock.
	 *
	 * @note this is called automatically for you when you register the entity and does not need to be called manually.
	 *
	 * @return true if the discovery message is successfully published, false otherwise.
	 */
  bool publishDiscovery();

	/**
	 * @brief Publishes the availability message for the lock.
	 *
	 * @note this is called automatically for you when you register the entity and call publishAvailabilities().
	 *
	 * @return true if the availability message is successfully published, false otherwise.
	 */
  bool publishAvailability();

	/**
	 * @brief Updates the state of the lock.
	 *
	 * This method updates the state of the lock to the specified value.
	 *
	 * @param val The new state of the lock (Lock::UNLOCKED, Lock::UNLOCKING, Lock::LOCKED, Lock::LOCKING, Lock::JAMMED).
	 * @return true if the state is successfully updated, false otherwise.
	 */
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

/**
 * @class Cover
 * @brief Represents a cover entity in Home Assistant.
 *
 * Usage:
 * - Create an instance of Cover with a name, display name, MQTT_HASS instance, device information, and callback pointer.
 * - Register the cover with the MQTT_HASS instance using registerEntity().
 * - Update the cover state using updateState() with the desired value.
 * - Handle cover actions using the specified callback pointer.
 *
 * @note This class simplifies the integration of covers with Home Assistant by providing a unified
 *  		 interface for discovery, state updates, and availability management.
 */
class Cover : public Entity {
public:

	/**
	 * @brief Enumerates the possible states of a cover entity.
	 */
  enum States {
    OPEN,    /**< Open */
    CLOSED,  /**< Closed */
    OPENING, /**< Opening */
    CLOSING, /**< Closing */
    STOPPED, /**< Stopped */
    __STATES_MAX,
  };

	/**
	 * @brief Enumerates the possible device classes for a cover entity.
	 */
  enum DeviceClasses {
    None,    /**< No device class */
    awning,  /**< Awning cover */
    blind,   /**< Blind cover */
    curtain, /**< Curtain cover */
    damper,  /**< Damper cover */
    door,    /**< Door cover */
    garage,  /**< Garage cover */
    gate,    /**< Gate cover */
    shutter, /**< Shutter cover */
    window,  /**< Window cover */
    __DEVICE_CLASSES_MAX,
  };

  Cover() = delete;

	/**
	 * @brief Constructs a Cover object with the given parameters.
	 *
	 * This constructor initializes a Cover object with the provided name, display name,
	 * MQTT_HASS instance, device information, callback pointer, and device class.
	 *
	 * @param name The name of the cover. (DO NOT USE ANY SPACES)
	 * @param displayName The display name of the cover.
	 * @param client A reference to the MQTT_HASS instance.
	 * @param dev The device information for the cover.
	 * @param callbackPtr A pointer to the callback function for handling cover actions.
	 * @param deviceClass The device class for the cover. (default None)
	 *
	 * @return A Cover object with the specified parameters.
	 */
  Cover(const String name, const String displayName, MQTT_HASS &client, Device dev, void (*callbackPtr)(char*, uint8_t*, unsigned int),
        DeviceClasses deviceClass = DeviceClasses::None);

	/**
	 * @brief Publishes the discovery message for the cover.
	 *
	 * @note this is called automatically for you when you register the entity and does not need to be called manually.
	 *
	 * @return true if the discovery message is successfully published, false otherwise.
	 */
  bool publishDiscovery();

	/**
	 * @brief Publishes the availability message for the cover.
	 *
	 * @note this is called automatically for you when you register the entity and call publishAvailabilities().
	 *
	 * @return true if the availability message is successfully published, false otherwise.
	 */
  bool publishAvailability();

	/**
	 * @brief Updates the state of the cover.
	 *
	 * This method updates the state of the cover to the specified value.
	 *
	 * @param val The new state of the cover (Cover::OPEN, Cover::CLOSED, Cover::OPENING, Cover::CLOSING, Cover::STOPPED).
	 *
	 */
  bool updateState(States val);

private:
  DeviceClasses deviceClass_;
  const char* states2Str[__STATES_MAX] = {
    "open",
    "closed",
    "opening",
    "closing",
    "stopped",
  };

  const char* deviceClasses2Str[__DEVICE_CLASSES_MAX] = {
    "None",
    "awning",
    "blind",
    "curtain",
    "damper",
    "door",
    "garage",
    "gate",
    "shutter",
    "window",
  };
};

