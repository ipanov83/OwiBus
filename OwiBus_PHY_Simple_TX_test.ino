#include <OwiBusPHY.h>
owibusphy phy(12);

uint8_t txPacket[20];
uint8_t rxPacket[20];
uint32_t okPackets = 0;
uint32_t errPackets = 0;
uint32_t timeoutPackets = 0;
uint32_t timer;

void setup() {
  Serial.begin(115200);
  phy.begin();
  for (uint8_t i = 1; i < 20; i++) txPacket[i] = i;
}

void loop() {
  uint32_t now = micros();
  static uint8_t counter = 0;
  txPacket[0] = counter++;
  phy.write(txPacket, 20);
  uint32_t start = millis();
  uint8_t index = 0;
  while (index < 20 && millis() - start < 20) {
    if (phy.available()) {rxPacket[index++] = phy.read();}
  }
  if (index != 20) {timeoutPackets++;}
  else {bool ok = true;
    for (uint8_t i = 0; i < 20; i++) {
      if (txPacket[i] != rxPacket[i]) {ok = false; break;}
    }
    if (ok) okPackets++;
    else    errPackets++;
  }
  
  if (now - timer > 1000000) {
    timer = now;
    Serial.print("OK/Err/TO: ");
    Serial.print(okPackets);
    Serial.print(" : ");
    Serial.print(errPackets);
    Serial.print(" : ");
    Serial.println(timeoutPackets);
  }
}
