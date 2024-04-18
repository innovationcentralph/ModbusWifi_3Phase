void printUint16ArrayHex(uint16_t* dataArray, size_t size, String label) {
  Serial.print(label + ": ");
  for (size_t i = 0; i < size; i++) {
    // Print each element (two bytes) in the array in hex
    Serial.print((dataArray[i] >> 8), HEX);
    Serial.print(' ');
    Serial.print((dataArray[i] & 0xFF), HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void ModbusMonitor_Routine( void * pvParameters ){

  Serial.print("Modbus monitoring running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    modbus_master.task();
    delay(2);
  } 
  
}

void uint16ToUint8Array(uint16_t value, uint8_t* byteArray) {
  // Split the 16-bit value into two 8-bit values
  byteArray[0] = value & 0xFF;          // Low byte
  byteArray[1] = (value >> 8) & 0xFF;   // High byte
}

float combineAndConvert(uint16_t valueA, uint16_t valueB) {
  

  union FloatConverter{
    uint8_t byteArray[4]; 
    float floatValue;
  }converter;

  uint16ToUint8Array(valueB, converter.byteArray);
  uint16ToUint8Array(valueA, converter.byteArray + 2);

  // Combine the bytes into a single float
//  Serial.println("BYTE VALUES: ");
//  for(int x = 0; x < 4; x++){
//    Serial.print(converter.byteArray[x], HEX); Serial.println("\t");
//  }
//  Serial.println();

  return converter.floatValue;
}
