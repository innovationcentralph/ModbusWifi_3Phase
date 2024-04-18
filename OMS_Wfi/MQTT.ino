void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String _topic = String(topic);

  String msg = "";
  
  for (int i = 0; i < length; i++) 
    msg = msg + (char)payload[i];

  #ifdef DEBUG_M
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println(msg);
    Serial.println(F("-----------------------"));
  #endif

  
  delay(50);

  if(_topic.indexOf(TOPIC_CMD) > -1){
    if(msg.indexOf("PING") > -1){
      mqttClient.publish(TOPIC_GENERAL, MSG_PING); 
    }
  }
  
}

void initMQTT(){
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

boolean connectToMQTT(){
    //Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(DEVICE_ESN, mqttUser, mqttPassword )) {
      #ifdef DEBUG_L
        Serial.println("Connected to MQTT");  
      #endif
      mqttClient.publish(TOPIC_GENERAL, MSG_INIT_CONNECT);
      subscribeMQTT();
    }

     return mqttClient.connected();
}
uint32_t lastReconnectAttempt = 0;
void checkMQTTConnectivity(){
//  if(!mqttClient.connected()){
//    if(WiFi.status() == WL_CONNECTED){
//      connectToMQTT();
//    } 
//  }

  if (!mqttClient.connected()) {
    
    if (millis() - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = millis();
      // Attempt to reconnect
      if (connectToMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    mqttClient.loop();
  }
}

void subscribeMQTT(){
  mqttClient.subscribe(TOPIC_CMD);
}
