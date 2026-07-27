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

#include <OwiBusPHY.h>
#include <avr/interrupt.h>
#include <Arduino.h>

owibusphy *owibusphy::active_object = 0;

volatile uint8_t owibusphy::_rx_buffer[_SS_MAX_RX_BUFF];
volatile uint8_t owibusphy::_tx_buffer[_SS_MAX_TX_BUFF];

volatile uint8_t owibusphy::_rx_tail = 0;
volatile uint8_t owibusphy::_rx_head = 0;
volatile uint8_t owibusphy::_tx_head = 0;
volatile uint8_t owibusphy::_tx_tail = 0;

volatile uint8_t owibusphy::_rx_bit  = 0;
volatile uint8_t owibusphy::_rx_byte = 0;
volatile uint8_t owibusphy::_tx_bit  = 0;
volatile uint8_t owibusphy::_tx_byte = 0;



owibusphy::owibusphy(uint8_t dataPin) : _buffer_overflow(false) {
  _dataPin        = dataPin;
  uint8_t _port   = digitalPinToPort(_dataPin);
  _dataBitMask    = digitalPinToBitMask(_dataPin);
  _portReg        = portOutputRegister(_port);
  _ddrReg         = portModeRegister(_port);
  _rxPortRegister = portInputRegister(_port);
  _txMask         = _dataBitMask;
  _txInvMask      = ~_dataBitMask;
  _state          = IDLE;
}

void owibusphy::begin() {

  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= _BV(WGM21);
  TCCR2B |= _BV(CS21);
  OCR2A = DETECT_BIT;
  TCNT2 = 0;
  TIFR2 |= _BV(OCF2A);
  TIMSK2 &= ~_BV(OCIE2A);

  *digitalPinToPCICR(_dataPin) |= _BV(digitalPinToPCICRbit(_dataPin));
  _pcint_maskreg   = digitalPinToPCMSK(_dataPin);
  _pcint_maskvalue = _BV(digitalPinToPCMSKbit(_dataPin));
  active_object = this;
  _PortToRX();
}

void owibusphy::_PortToTX() {

  if (!(*_portReg & _txMask)) { *_portReg |= _txMask;}    // idle HIGH
  if (!(*_ddrReg & _txMask))  { *_ddrReg |= _txMask;}     // OUTPUT
  setRxIntMsk(false);
}

void owibusphy::_PortToRX() {
  if (*_ddrReg & _txMask)     {*_ddrReg &= _txInvMask;}   // INPUT
  if (!(*_portReg & _txMask)) { *_portReg |= _txMask;}    // pull-up
  setRxIntMsk(true);
}

void owibusphy::setRxIntMsk(bool enable) {
  if (enable) *_pcint_maskreg |= _pcint_maskvalue;
  else        *_pcint_maskreg &= ~_pcint_maskvalue;
}


ISR(PCINT0_vect){
  owibusphy *obj = owibusphy::active_object;
  if (!obj)                                      return;
  if (obj->_state != IDLE)                       return;
  if (*obj->_rxPortRegister & obj->_dataBitMask) return;
  obj->setRxIntMsk(false);
  obj->_state = RX_START;
  TCNT2 = 0;
  OCR2A = DETECT_BIT;
  TIFR2 |= _BV(OCF2A);
  TIMSK2 |= _BV(OCIE2A);
}


ISR(TIMER2_COMPA_vect) {

  owibusphy *obj = owibusphy::active_object;
  if (!obj) return;

  if (obj->_state <= RX_RELEASE) {
    uint8_t pin = *obj->_rxPortRegister;

    if (obj->_state == RX_START) {
      if (pin & obj->_dataBitMask) {
        TIMSK2 &= ~_BV(OCIE2A);
        TIFR2 |= _BV(OCF2A);
        TCNT2 = 0;
        obj->_state = IDLE;
        obj->setRxIntMsk(true);
        return;
      }
      obj->_rx_byte = 0;
      obj->_rx_bit =  0;
      OCR2A = BITS_TICKS;
      obj->_state = RX_DATA;
      return;
    }

    if (obj->_state == RX_DATA) {
      if (pin & obj->_dataBitMask) {
        obj->_rx_byte |= (1 << obj->_rx_bit);
      }
      obj->_rx_bit++;
      if (obj->_rx_bit >= 8)  {obj->_state = RX_RELEASE;}
      return;
    }

    if (obj->_state == RX_RELEASE) {
      TIMSK2 &= ~_BV(OCIE2A);
      TIFR2 |= _BV(OCF2A);
      TCNT2 = 0;
      obj->storeRxByte(obj->_rx_byte);
      obj->_state = IDLE;
      obj->setRxIntMsk(true);
      return;
    }
  }

  if (obj->_state == TX_DATA) {
    if (obj->_tx_byte & 1) {*obj->_portReg |= obj->_txMask;}
    else {*obj->_portReg &= ~obj->_txMask;}
    obj->_tx_byte >>= 1;
    obj->_tx_bit++;
    if (obj->_tx_bit >= 9) {obj->_state = TX_RELEASE;}      // +1bit time for turnaround Tx->Rx (NOT BIT, NOT CHANGING !!!)
    return;
  }

  if (obj->_state == TX_RELEASE) {
    TIMSK2 &= ~_BV(OCIE2A);
    TIFR2 |= _BV(OCF2A);
    TCNT2 = 0;
    obj->_PortToRX();
    obj->_state = IDLE;
    if(owibusphy::txAvailable()) {obj->startTX();}
    return;
  }

}

void owibusphy::storeRxByte(uint8_t b){
  uint8_t next = (_rx_tail + 1)  &  (_SS_MAX_RX_BUFF - 1);
  if (next != _rx_head) {
    _rx_buffer[_rx_tail] = b;
    _rx_tail = next;}
  else {_buffer_overflow = true;}
}

uint8_t owibusphy::read() {
  if (_rx_head == _rx_tail) return false;
return _rx_buffer[_rx_head++ & (_SS_MAX_RX_BUFF - 1)];

}

uint8_t owibusphy::available() {
  return (_rx_tail + _SS_MAX_RX_BUFF - _rx_head) & (_SS_MAX_RX_BUFF - 1);
}

void owibusphy::startTX(){
  if (_state != IDLE) {return;}
  if (!(owibusphy::txAvailable())) {return;}
  _state = TX_DATA;
  _PortToTX();
  _tx_byte = _tx_buffer[_tx_tail];
  _tx_tail = (_tx_tail+1)&(_SS_MAX_TX_BUFF-1);
  _tx_bit = 0;
  *_portReg &= ~_txMask;
  TCNT2   = 0;
  OCR2A   = BITS_TICKS;
  TIFR2  |= _BV(OCF2A);
  TIMSK2 |= _BV(OCIE2A);
}

size_t owibusphy::write(const char *str){
  return write((const uint8_t*)str, strlen(str));
}

size_t owibusphy::write(const uint8_t *buffer, int size) {
  uint8_t written = 0;
  while(written < size) {
    uint8_t next = (_tx_head + 1) & (_SS_MAX_TX_BUFF - 1);
    if(next == _tx_tail) {
      if(_state == IDLE) {startTX();}
    continue;}
    _tx_buffer[_tx_head] = buffer[written++];
    _tx_head = next;
    if(_state == IDLE) startTX();
  }
  return written;
}

size_t owibusphy::write(uint8_t b){

  uint8_t next = (_tx_head + 1) & (_SS_MAX_TX_BUFF - 1);
  if (next == _tx_tail) {return 0;}
  _tx_buffer[_tx_head] = b;
  _tx_head = next;
  if(_state == IDLE) {startTX();}
  return 1;
}

void owibusphy::flush(){
  while(_state != IDLE || owibusphy::txAvailable()) {
    if(_state == IDLE) {startTX();}
  }
}

void owibusphy::flushRx(){
    _rx_head = 0;
    _rx_tail = 0;
}

bool owibusphy::overflow(){
    bool ret = _buffer_overflow;
    _buffer_overflow = false;
    return ret;
}
