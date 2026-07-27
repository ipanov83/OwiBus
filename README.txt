OwiBusPHY v1.0.0
by Ivaylo Panov — 27.07.2026
Interrupt-driven half-duplex single-wire physical layer for AVR microcontrollers

OwiBusPHY is a lightweight software-based physical layer driver designed for reliable half-duplex communication over a single GPIO line on AVR microcontrollers.

The driver implements an asynchronous serial communication interface without requiring a dedicated UART peripheral. It uses Pin Change Interrupts for receive start detection and Timer2 compare interrupts for accurate bit timing, providing a deterministic and efficient communication layer suitable for embedded systems.

The main goal of OwiBusPHY is to provide a simple, fast and resource-efficient communication method for applications where:

    hardware UART peripherals are unavailable,

    UART resources are already occupied,

    a custom single-wire bus is required,

    deterministic timing and low CPU overhead are important.

OwiBusPHY does not block the processor during communication and allows the main application to continue running while data is transmitted or received.

---

# Features

- Half-duplex communication over a single wire
- Single GPIO bus operation
- Software UART implementation
- 8 data bits, no parity, no stop bit (**8N0 compatible timing**)
- Configurable communication speed through timer bit timing
- Interrupt-driven reception
- Timer-driven RX and TX timing
- Automatic TX/RX direction switching
- Separate RX and TX ring buffers
- Direct register GPIO access for maximum speed
- Low interrupt overhead
- Low RAM usage
- Deterministic execution timing

---

# Communication Format

OwiBusPHY uses an asynchronous serial format:

8N0


Meaning:

8 data bits
No parity bit
No stop bit


The missing stop bit is intentional. The driver provides a controlled turnaround interval between transmission and reception, allowing reliable half-duplex operation on a shared line.

This approach reduces transmission overhead and improves bus efficiency.

# Why 8N0?

Traditional UART requires a stop bit after every byte.
OwiBusPHY removes this overhead because direction control and turnaround timing are handled by the protocol layer.

This makes the bus up to 10% reduction in byte overhead for short embedded packets.

---

# Supported Bit Timing

The driver uses Timer2 compare interrupts for precise bit timing.

Typical tested configurations:

| Bit timing | Approximate speed |
|------------|------------------|
| BITS_TICKS = 34 | ~57600 baud |
| BITS_TICKS = 44 | ~38400 baud |
| BITS_TICKS = 51 | ~31250 baud |
| BITS_TICKS = 63 | ~19200 baud |

The optimal value depends on:

- CPU frequency
- interrupt load
- cable length
- electrical characteristics of the bus

The driver has been tested successfully on AVR running at 16 MHz.

---

# Internal Design

OwiBusPHY uses a minimal interrupt architecture:

### RX start detection

Receive activity is detected using:

- Pin Change Interrupt (PCINT)

When the bus goes LOW, reception begins and Timer2 is configured to sample incoming bits.

---

### Bit timing

Timer2 Compare Interrupt handles:

- RX sampling
- TX bit generation
- TX/RX turnaround timing

Direct register access is used instead of Arduino GPIO functions to minimize execution time.

---

### Direction control

The communication line is switched automatically:

TX mode
|
| transmit packet
|
turnaround delay
|
RX mode
|
| listen to bus


This allows multiple devices to share the same single-wire bus.

---

# Memory and Performance

The driver intentionally uses:

- Static buffers
- Static ISR state
- Single active instance

These choices are deliberate.

Advantages:

- predictable ISR execution time
- reduced memory usage
- stable timing
- minimal RAM fragmentation

The design targets small AVR devices where memory and timing stability are more important than object flexibility.

---

# Hardware Requirements

Required:

- AVR microcontroller with Pin Change Interrupt support
- One GPIO pin connected to the bus
- Common GND connection

Recommended:

- External pull-up resistor for longer cables
- Proper bus wiring for multi-node systems

Example tested boards:

- Arduino Uno (ATmega328P)
- Arduino Mega / Mega 2560 (ATmega2560)

Tested communication pins:

- GPIO 12/13

Other AVR devices may work with minor adjustments.

---

# Limitations

## Interrupt usage

OwiBusPHY currently uses:

- AVR Pin Change Interrupt vector **PCINT0**
- Hardware Timer2 Compare Interrupt

Because of this, some Arduino peripherals may conflict with the driver.

### Timer2 conflicts

Timer2 is used exclusively for bit timing generation and sampling.

The following Arduino functions may conflict:

- "tone()" - command
- PWM output on Arduino pin 3 and pin 11 (ATmega328P)
- Other libraries using Timer2

## Arduino Uno / Nano (ATmega328P)

Supported pins:

| Arduino pin | AVR port | Interrupt |
|-------------|----------|-----------|
| D8  | PB0 | PCINT0 |
| D9  | PB1 | PCINT1 |
| D10 | PB2 | PCINT2 |
| D11 | PB3 | PCINT3 |
| D12 | PB4 | PCINT4 |
| D13 | PB5 | PCINT5 |

## Arduino Mega / Mega 2560 (ATmega2560)

Supported pins:

| Arduino pin | AVR port | Interrupt |
|-------------|----------|-----------|
| D53 | PB0 | PCINT0 |
| D52 | PB1 | PCINT1 |
| D51 | PB2 | PCINT2 |
| D50 | PB3 | PCINT3 |
| D10 | PB4 | PCINT4 |
| D11 | PB5 | PCINT5 |
| D12 | PB6 | PCINT6 |
| D13 | PB7 | PCINT7 |

Only one OwiBusPHY instance is supported because the current implementation uses a single shared PCINT0 interrupt handler.
---

# Reliability Testing

## Stress test

Bidirectional closed-loop communication test.

Both nodes validate communication:

TX node: Atmega 2560 at 16 MHz, uses pin 12 to communication

- makes packet on startup
- sends packet
- receives response
- compares returned data
- show results

RX node: Atmega 328P-PU on breadboard at 16MHz, uses pin 13 to communication

- validates incoming packet
- generates response packet

Test sketches:

TX:
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


RX:
#include <OwiBusPHY.h>
owibusphy phy(13);
uint8_t packet[20];

void setup() {
  phy.begin();
}

void loop() {
  if (phy.available() >= 20) {
    for (uint8_t i = 0; i < 20; i++) {packet[i] = phy.read();}
    phy.write(packet, 20);
  }
}
---

Packet size: 20 bytes in range from 0x00 to 0xFF, generated from TX on startup.

The Rx device must be resetted BEFORE starting the Tx device, during test - synchronisation.

Test result:

Packets exchanged: 999492 (19 989 840 bytes)


Serial monitor at 115200 output:
...
OK/Err/TO: 999177 : 4 : 0
OK/Err/TO: 999282 : 4 : 0
OK/Err/TO: 999387 : 4 : 0
OK/Err/TO: 999492 : 4 : 0
...

Meaning:

999492 packets x20 bytes transmitted
4 errors (not equal packets)
0 timeouts

Observed error rate: ~0.0004 %

Communication remained stable after errors.
No synchronization loss observed.

---

# Example Applications

Possible applications:

- Sensor networks
- Home automation
- Embedded controllers
- Multi-node AVR systems
- Custom industrial buses
- Simple single-wire device networks
- Communication between small microcontrollers

---

# Project Philosophy

OwiBusPHY is not intended to replace high-performance communication peripherals.

Instead, it provides a lightweight physical communication layer for projects where:

- simplicity matters
- hardware resources are very limited
- deterministic timing is required
- a custom single-wire bus is preferred

The implementation favors reliability and predictable behavior over abstraction.

---

# License

MIT License

Copyright (c) 2026 Ivaylo Panov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files, to deal in the Software
without restriction.

The software is provided "AS IS", without warranty of any kind.

---

# OwiBusPHY

A small AVR communication layer built for practical embedded projects.

Version 1.0
