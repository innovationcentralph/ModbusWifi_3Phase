#include <ArduinoRS485.h> 
#include <ModbusRTU.h>
#include <HardwareSerial.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Local dependencies
#include "SystemConfig.h"
#include "UserConfig.h"
#include "HTTPSClient.h"


// Instance creation
HardwareSerial modbusMaster(2);
ModbusRTU modbus_master;
TaskHandle_t ModbusMonitor;

WiFiManager wm;

WiFiClient   espClient;
PubSubClient mqttClient(espClient);

HTTPSClient HTTPSClient; 

uint8_t METER_ID[NUMBER_OF_METERS] = {1, 3, 5};


// Global variables
uint32_t heartBeatMillis = 0;
uint32_t wifiMillis = 0;
uint32_t lastConnectionTime = 0;


uint32_t dataSendingInterval = 0;

void setup() {
  
  Serial.begin(9600);
  modbusMaster.begin(9600, SERIAL_8N1, MODBUS_RO, MODBUS_DI);
  
  delay(1000);

  pinMode(WDT_DONE, OUTPUT);
  digitalWrite(WDT_DONE, LOW);

  pinMode(STATUS_INDICATOR, OUTPUT);
  digitalWrite(STATUS_INDICATOR, LOW);


  


  #ifdef DEBUG_L
    Serial.println(F("\n- -  3PHASE MODBUS MONITOR  - - - -"));
    Serial.println(F("- - - - Initializing System - - - -"));
  #endif

  
  //wm.resetSettings();
  wm.setConfigPortalTimeout(120);
  bool res = wm.autoConnect("IOT-OMS V2.0","innovation");// password protected ap
  
  
  if(!res) {
    #ifdef DEBUG_L
      Serial.println("Failed to connect");
    #endif
    ESP.restart();
  } 
  else {
    #ifdef DEBUG_L   
      Serial.println("Connected to network");
    #endif

    initMQTT();

    connectToMQTT();
    
  }
  
  WiFi.mode(WIFI_STA);
  wm.setWiFiAutoReconnect(true);

  dataSendingInterval = DEFAULT_HEARTBEAT_INTERVAL;
  dataSendingInterval = dataSendingInterval * 1000; // convert to seconds

  #ifdef DEBUG_L
    Serial.println("HEART INTERVAL: " + String(dataSendingInterval) + " millisecconds");
  #endif

  
  /* Initialize Modbus Master and Slave */
  modbus_master.begin(&modbusMaster, MODBUS_EN);
  modbus_master.setBaudrate(9600);
  modbus_master.master();

  xTaskCreatePinnedToCore(ModbusMonitor_Routine, "Modbus Monitor", 1000, NULL, 1, &ModbusMonitor, 0);     // Default loop runs on Core 1
  delay(500);

}

int httpCode = 0;
String URL = "";
String httpsResponse = "";

uint32_t previousMillis = 0;
float PhaseCurrent[3];
float ActiveEnergy = 0;

void loop() {

  

  if(millis() - previousMillis > MODBUS_READ_INTERVAL){
    
    uint16_t currentReading[6] = {0};
    modbus_master.readHreg((uint8_t)1, (uint16_t)270, currentReading, 6);
    delay(800);

    // Convert uint16_t values to float in ABCD format
    
    PhaseCurrent[0] = combineAndConvert(currentReading[0], currentReading[1]);
    PhaseCurrent[1] = combineAndConvert(currentReading[2], currentReading[3]);
    PhaseCurrent[2] = combineAndConvert(currentReading[4], currentReading[5]);

    uint16_t energyReading[2] = {0};
    modbus_master.readHreg((uint8_t)1, (uint16_t)316, energyReading, 2);
    delay(800);

    ActiveEnergy = combineAndConvert(energyReading[0], energyReading[1]) - OffsetValue;

    Serial.println("Current Line 1: " + String(PhaseCurrent[0], 2) + "A");
    Serial.println("Current Line 2: " + String(PhaseCurrent[1], 2) + "A");
    Serial.println("Current Line 3: " + String(PhaseCurrent[2], 2) + "A");
    
    Serial.println("Accumulated Energy: " + String(ActiveEnergy, 3) + "W");
        
    previousMillis = millis();
    
  }

  if(millis() - heartBeatMillis > DEFAULT_HEARTBEAT_INTERVAL){
    https://innovationcentralph.com/EnergyMonitor/resources/data/APIsensorlog.php?I1=1.1&I2=4.5&I3=3.2&E=8.1&id=ABC123
    String URL = "https://innovationcentralph.com/EnergyMonitor/resources/data/APIsensorlog.php?I1=" + String(PhaseCurrent[0], 2) + "&I2=" + String(PhaseCurrent[1], 2) + "&I3=" + String(PhaseCurrent[2], 2) + "&E=" + String(ActiveEnergy, 2) + "&id=ABC123";
    Serial.println("URL: " + URL);

    httpsResponse = HTTPSClient.SendGetRequest(URL, false, httpCode);
    delay(100);
    Serial.println("HTTP RESPONSE: " + String(httpsResponse));
    
    heartBeatMillis = millis();
  }

  

  /* Handle WiFi */
  if (WiFi.status() == WL_CONNECTED) {
    if(mqttClient.connected()){
      digitalWrite(STATUS_INDICATOR, HIGH);
    }
    lastConnectionTime = millis();
  }
  else{
    digitalWrite(STATUS_INDICATOR, LOW);
    if(millis() - lastConnectionTime > 5000){
      ESP.restart();
    }
  }

  /* Handle MQTT */
  checkMQTTConnectivity();
  

}
