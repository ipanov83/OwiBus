/* OwiBusPHY version 1.0.0 from 27.07.2026

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

#pragma once
#include <Arduino.h>

#define BITS_TICKS 44   //34, 51, 63, 103 - (~57600, ~38400, ~31250, ~19200) (34-15, 40-18, 44-20, 51-23)
#define DETECT_BIT 20

#define _SS_MAX_RX_BUFF 64
#define _SS_MAX_TX_BUFF 64

#define RX_START    0
#define RX_DATA     1
#define RX_RELEASE  2
#define TX_DATA     3
#define TX_RELEASE  4
#define IDLE        5

class owibusphy {

private:

  uint8_t _dataPin;
  volatile uint8_t _buffer_overflow : 1;
  static volatile uint8_t _rx_buffer[_SS_MAX_RX_BUFF];
  static volatile uint8_t _rx_tail;
  static volatile uint8_t _rx_head;
  static volatile uint8_t _tx_buffer[_SS_MAX_TX_BUFF];
  static volatile uint8_t _tx_head;
  static volatile uint8_t _tx_tail;

public:

  owibusphy(const uint8_t dataPin = 13);
  static owibusphy *active_object;
  static bool txAvailable() {return _tx_head != _tx_tail;}
  static volatile uint8_t _rx_bit;
  static volatile uint8_t _rx_byte;
  static volatile uint8_t _tx_bit;
  static volatile uint8_t _tx_byte;
  volatile uint8_t *_portReg;
  volatile uint8_t *_ddrReg;
  volatile uint8_t *_pcint_maskreg;
  volatile uint8_t *_rxPortRegister;
  volatile uint8_t _state;
  uint8_t _dataBitMask;
  uint8_t _pcint_maskvalue;
  uint8_t _txMask;
  uint8_t _txInvMask;
  uint8_t available();
  uint8_t read();
  inline __attribute__((always_inline)) void _PortToTX();
  inline __attribute__((always_inline)) void _PortToRX();
  inline __attribute__((always_inline)) void startTX();
  inline __attribute__((always_inline)) void storeRxByte(uint8_t b);
  inline __attribute__((always_inline)) void setRxIntMsk(bool enable);
  void begin();
  void flush();
  void flushRx();
  size_t write(uint8_t byte);
  size_t write(const uint8_t *buffer, int size);
  size_t write(const char *str);
  bool overflow();
  operator bool() {return true;}
};
