#include "OwiBus.h"
OwiBus bus(12);

uint32_t loopCounter, startTime, idleCounter = 152871; //лупове на празен ход измерени експериментално
float CPUload;

uint32_t tx, rx;
uint16_t err, nack;

uint32_t lastSend = 0;
uint32_t lastReport = 0;

uint32_t sendTime = 0;

uint8_t seq = 0;
uint32_t t, f, r;
uint8_t lastSeq = 0; 

void setup()
{
  Serial.begin(115200);

  bus.begin();

  Serial.println("OWIBUS MASTER TEST");
}



void loop(){

uint32_t now = micros();
loopCounter++;
  
  bus.update();
  
  if(now - lastSend > 100000) {lastSend = now;

   // sendTime = now;
    bus.send(2, 0x10,"11112345678901234567890123456789012345678901234567890", ACK_REQ);
   // r = micros();
   // t = r - sendTime;
    tx++;
  }

  if(bus.available()) {
    rx++;
  //  f = micros()-r;
    if (!bus.ackReceived()){nack++;}
    uint8_t s = bus.sequence();
    int8_t diff = (int8_t)(s - lastSeq);
    if (diff != 1){
      if (!(lastSeq == 255 && s == 1)) {err++;}
    }
    lastSeq = s;
  }


  if(now-lastReport > 1000000)
  {
    lastReport = now;
    
  //  Serial.print("uS Sn/Rp: ");
 //   Serial.print(t);
  //  Serial.print(":");
 //   Serial.println(f);
    Serial.print("Tx/Rx/Err/NACK: ");
    Serial.print(tx);
    Serial.print(":");
    Serial.print(rx);
    Serial.print(":");
    Serial.print(err);     
    Serial.print(":");
    Serial.print(nack); 
    CPUload = ((float)(idleCounter - loopCounter) * 100.0f) / (float)idleCounter;    
    //Serial.print("Loops:");
    //Serial.println(loopCounter);
    Serial.print(" CPU:");
    Serial.print(CPUload);
    Serial.println("%");
    loopCounter = 0;

  }

}
