/* OwiBus version 1.0.0 from 27.07.2026

Copyright (c) 2026 Ivaylo Panov

Hardware limitation: only one active OwiBusPHY instance is supported.
Static buffers and state are intentional for ISR performance.


MIT License

Copyright (c) 2026 Ivaylo Panov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "OwiBus.h"

OwiBus::OwiBus (uint8_t pin): _port(pin) {

    _deviceId = 0;
    _state = WAIT_SYNC;
    _packetReady = false;
    _ackReceived = false;
    _txSequence = _rxSequence = 0;
    _calc = _rxCrc = 0;
    _rxIndex = _rxLength = 0;
    _payload[0] = 0;
}

void OwiBus::begin(uint8_t deviceId) {
  _port.begin();
  _deviceId = deviceId;

}

uint8_t OwiBus::crc8(uint8_t crc, uint8_t data) {
  for (uint8_t i = 8; i; i--) {
    uint8_t mix = (crc ^ data) & 0x01;
    crc >>= 1;
    if(mix) {crc ^= 0x8C;}
    data >>= 1;
  }
  return crc;
}

bool OwiBus::send(uint8_t receiver, uint8_t command, const char *text, uint8_t flags) {

  uint8_t len = strlen(text);
  if (len > TXT_BUFFER) {len = TXT_BUFFER;}
  uint8_t packet[TXT_BUFFER + 9];
  uint8_t crc = 0;

  packet[0] = SYNC_START;
  packet[1] = receiver;    crc = crc8(crc, packet[1]);
  packet[2] = _deviceId;   crc = crc8(crc, packet[2]);

  if (_deviceId == MASTER_ID) {_txSequence++;
    if (!_txSequence) {_txSequence = 1;}
    packet[3] = _txSequence;}
  else  {packet[3] =_rxSequence;}

                           crc = crc8(crc, packet[3]);
  packet[4] = flags;       crc = crc8(crc, packet[4]);
  packet[5] = command;     crc = crc8(crc, packet[5]);
  packet[6] = len;         crc = crc8(crc, packet[6]);


  for(uint8_t i = 0; i < len; i++){
    packet[7 + i] = text[i];
    crc = crc8(crc,text[i]);
  }

  packet[7 + len] = crc;
  packet[8 + len] = SYNC_STOP;
  _port.write(packet,len + 9);
  return true;
}

void OwiBus::update() {
  
  uint8_t count = 0;
  while (_port.available() && count < 64) {
    uint8_t b = _port.read();

    if (_state == WAIT_SYNC) {
      if (b == SYNC_START) {
        _bytetime = micros();
        _calc = 0;
        _state = WAIT_RECEIVER;}
    return;}

    if (_state == WAIT_RECEIVER) {
      if (b == _deviceId || b == SEND_TO_ALL) {
        _payload[0] = '\0';
        _rxIndex = _rxLength = _rxCrc = 0;
        _calc = crc8(_calc, b);
        _bytetime = micros();
        _rxReceiver = b;
        _state = WAIT_SENDER;}
      else {
        _packetReady = false;
        _state = WAIT_IGNORE;}
    return;}

    if (_state == WAIT_SENDER) {
      _calc = crc8(_calc, b);
      _rxSender = b;
      _state = WAIT_SEQUENCE;
    return;}

    if (_state == WAIT_SEQUENCE) {
      _calc = crc8(_calc, b);
      _rxSequence = b;
      _state = WAIT_FLAGS;
    return;}

    if (_state == WAIT_FLAGS) {
      _calc = crc8(_calc, b);
      _rxFlags= b;
      if (_rxFlags == ACK_REPLY) {_state = WAIT_CRC;}         // продължава директно към црц ако е отговор на АСК
      else {_state = WAIT_COMMAND;}
    return;}

    if (_state == WAIT_COMMAND) {
      _calc = crc8(_calc, b);
      _rxCommand  = b;
      _state = WAIT_LENGTH;
    return;}

    if (_state == WAIT_LENGTH) {
      _calc = crc8(_calc, b);
      _rxLength   = b;
      b == 0 ?_state = WAIT_CRC : _state = WAIT_DATA;         // продължава директно към црц ако тук няма нищо
    return;}

    if (_state == WAIT_DATA) {
      _calc = crc8(_calc, b);
      if (_rxIndex < TXT_BUFFER) {_payload[_rxIndex] = b;}
      _rxIndex++;
      if (_rxIndex >= _rxLength) {
         if (_rxLength <= TXT_BUFFER) {_payload[_rxLength] = '\0';}
         else {_payload[TXT_BUFFER] = '\0';}
         _state = WAIT_CRC;}
    return;}

    if (_state == WAIT_CRC) {_rxCrc = b; _state = WAIT_END; return;}

    if (_state == WAIT_END) {
      if (b == SYNC_STOP) {
        if (_calc == _rxCrc) {
          if (_rxFlags == ACK_REPLY) {
            _ackReceived = true;
            _packetReady = false;}
          else {_packetReady = true;}
          if (_rxFlags == ACK_REQ) {
            sendACK(_rxSender, _rxSequence);
            _rxFlags = ACK_NONE; }
        }
        else {_packetReady = false;}
      }
      else {_packetReady = false;}
      _state = WAIT_SYNC;
    return;}

    if (_state == WAIT_IGNORE) {
      if (b == SYNC_STOP) {
        _state = WAIT_SYNC;
        _port.flushRx();}
      else if (b == SYNC_START) {
        _bytetime = micros();
        _state = WAIT_RECEIVER;}
    return;}

 count++;}

    if (_state != WAIT_SYNC) {
      if (micros() - _bytetime > 60000UL) {
        _packetReady = false;
        _state = WAIT_SYNC;}
    }

}

uint8_t OwiBus::sender()   {return _rxSender;}
uint8_t OwiBus::command()  {return _rxCommand;}
uint8_t OwiBus::sequence() {return _rxSequence;}
const char* OwiBus::text() {return _payload;}

bool OwiBus::available() {
  bool ready = _packetReady;
  _packetReady = false;
  return ready;
}

bool OwiBus::ackReceived() {
  bool ret = _ackReceived;
  _ackReceived = false;
  return ret;
}


bool OwiBus::sendACK(uint8_t receiver, uint8_t sequence) {

  if (receiver != MASTER_ID) {return false;}

  uint8_t packet[7];
  uint8_t crc = 0;

  packet[0] = SYNC_START;
  packet[1] = receiver;  crc = crc8(crc, packet[1]);
  packet[2] = _deviceId; crc = crc8(crc, packet[2]);
  packet[3] = sequence;  crc = crc8(crc, packet[3]);
  packet[4] = ACK_REPLY; crc = crc8(crc, packet[4]);
  packet[5] = crc;
  packet[6] = SYNC_STOP;
  _port.write(packet, 7);
  return true;
}
