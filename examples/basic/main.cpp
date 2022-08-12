#include <Arduino.h>
#include "JetiSerial.h"

void setup() {
  JETISerial JetiSerial;
  JetiSerial.begin(9600, 32);

  for(uint8_t i=0; ; i++) {
    JetiSerial.send(i, 0x01);
    if(i==255) i=0;
    vTaskDelay(10);
  }
}

void loop() {}
