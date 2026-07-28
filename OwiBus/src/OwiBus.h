#pragma once
#include <Arduino.h>
#include <OwiBusPHY.h>

#define TXT_BUFFER      53
#define MASTER_ID     0x00
#define SEND_TO_ALL   0xFF
#define SYNC_START    0x01
#define SYNC_STOP     0x02
#define ACK_REQ       0x03
#define ACK_REPLY     0x04
#define ACK_NONE      0x00

#define WAIT_SYNC     0x01
#define WAIT_RECEIVER 0x02
#define WAIT_SENDER   0x03
#define WAIT_SEQUENCE 0x04
#define WAIT_FLAGS    0x05
#define WAIT_COMMAND  0x06
#define WAIT_LENGTH   0x07
#define WAIT_DATA     0x08
#define WAIT_CRC      0x09
#define WAIT_END      0x0A
#define WAIT_IGNORE   0x0B

class OwiBus
{
public:

    OwiBus(uint8_t pin);

    void begin(uint8_t deviceId = MASTER_ID);

    bool send(uint8_t receiver = SEND_TO_ALL, uint8_t command = SEND_TO_ALL, const char *text = "", uint8_t flags = ACK_NONE);
    void update();
    bool available();
    bool ackReceived();

    uint8_t sender();
    uint8_t command();
    uint8_t sequence();
    const char *text();

private:

    owibusphy _port;

    uint32_t _bytetime;
    uint8_t _state;
    uint8_t _calc;
    uint8_t _rxReceiver;
    uint8_t _rxSender;
    uint8_t _rxCommand;
    uint8_t _txSequence;
    uint8_t _rxSequence;
    uint8_t _rxFlags;
    uint8_t _rxLength;
    uint8_t _rxIndex;
    uint8_t _rxCrc;
    uint8_t _deviceId;
    char _payload[TXT_BUFFER + 1];
    bool _packetReady;
    bool _ackReceived;
    bool sendACK(uint8_t receiver, uint8_t sequence);
    uint8_t crc8(uint8_t crc, uint8_t data);
};
