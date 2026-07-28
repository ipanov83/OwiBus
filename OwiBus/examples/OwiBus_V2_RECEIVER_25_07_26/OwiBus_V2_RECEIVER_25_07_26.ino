#include "OwiBus.h"
OwiBus bus(13);

uint32_t loopCounter, idleCounter = 198718; //лупове на празен ход измерени експериментално
float CPUload;

uint32_t rx=0;
uint32_t tx=0;

uint32_t lastReport,  lastSeq, nack; 

void setup()
{
  Serial.begin(115200);
  bus.begin(2);

  Serial.println("OWIBUS SLAVE TEST");
}



void loop() {
  uint32_t now = micros();
  loopCounter++;
  bus.update();


  if (bus.available()){  
    rx++;
    
    if (bus.command()==0x10) {
      uint8_t s = bus.sequence();
      int8_t diff = (int8_t)(s - lastSeq);
      if(diff != 1) {
        if(!(lastSeq == 255 && s == 1))  {nack++;}
    }

    lastSeq = s;
    bus.send(bus.sender(), 0x11, bus.text(), ACK_REQ);
    tx++;
    }
  }



  if (now - lastReport > 1000000) {
    lastReport = now;
    Serial.print("Tx/Rx: ");
    Serial.print(tx);
    Serial.print(":");
    Serial.print(rx);
    
    CPUload = ((float)(idleCounter - loopCounter) * 100.0f) / (float)idleCounter;    
 //   Serial.print("Loops:");
 //   Serial.println(loopCounter);
    Serial.print(" CPU:");
    Serial.print(CPUload);
    Serial.println("%");
    loopCounter = 0;
  }

}
