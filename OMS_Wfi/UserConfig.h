#define DEVICE_ESN "ICPM001"

#define DEFAULT_HEARTBEAT_INTERVAL 130000 // sending interval to web in seconds
#define MODBUS_READ_INTERVAL 5000

#define MODBUS_READ_DELAY 300

#define OffsetVal 39.95

//MQTT Parameters
#define mqttServer   "driver.cloudmqtt.com"
#define mqttPort     18933
#define mqttUser     "gnigrukf"
#define mqttPassword "4YkZBFFRAgIy"

#define TOPIC_GENERAL      "3PMODBUS/Main"
#define TOPIC_CMD          "3PMODBUS/001/CMD"

#define MSG_INIT_CONNECT   "3PMODBUS 001 Initialized"
#define MSG_PING           "3PMODBUS 001 Alive"

#define NUMBER_OF_METERS 3
