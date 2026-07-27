#include <OwiBusPHY.h>
owibusphy phy(13);      

uint8_t packet[20];

void setup() {
  phy.begin();                               // OwiBusPHY begin
}

void loop() {
  if (phy.available() >= 20) {
    for (uint8_t i = 0; i < 20; i++) {packet[i] = phy.read();}
    phy.write(packet, 20);
  }
}
