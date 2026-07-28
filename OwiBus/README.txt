# OwiBus v1.0.0

## Lightweight multi-node single-wire communication protocol for AVR microcontrollers

**OwiBus** is a lightweight communication protocol designed for reliable half-duplex multi-node communication between embedded devices.

It is built on top of **OwiBusPHY**, a software-based physical layer driver that provides the low-level single-wire communication using AVR GPIO, interrupts and hardware timer timing.

OwiBus provides a simple packet-based communication system with:

- device addressing
- multi-node support
- broadcast messages
- command based communication
- CRC8 error detection
- sequence tracking
- optional packet acknowledgement

The main goal of OwiBus is to provide a simple and efficient communication bus for embedded projects where:

- hardware UART resources are unavailable or already occupied
- multiple controllers need to exchange data
- a simple single-wire bus is preferred
- low memory usage and deterministic behavior are important


---

# Architecture

OwiBus is divided into two layers:

Application

 |
 |

OwiBus
Protocol layer

 |
 |

OwiBusPHY
Physical layer

 |
 |

Single Wire Bus


## OwiBusPHY

Responsible for:

- bit timing
- GPIO communication
- TX/RX switching
- interrupt driven communication


## OwiBus

Responsible for:

- packet structure
- addressing
- commands
- acknowledgements
- CRC validation
- multi-node communication


---

# Features

## Multi-node communication

OwiBus is designed as a shared bus system.

Each device has its own address and can communicate with other nodes on the same wire.

Example:

      Single Wire Bus

    +--------------+-------------+-------------+
 Node 1         Node 2         Node 3        Node 4
  ID 1           ID 2           ID 3          ID 4



Devices can send messages:

- to a specific node
- to all nodes using broadcast address


---

# Packet Format

Standard packet:

+-------------+
| SYNC_START |
+-------------+
| Receiver ID |
+-------------+
| Sender ID |
+-------------+
| Sequence |
+-------------+
| Flags |
+-------------+
| Command |
+-------------+
| Length |
+-------------+
| Payload |
+-------------+
| CRC8 |
+-------------+
| SYNC_STOP |
+-------------+



---

# Addressing

Each device has a unique ID.

Default master address:

0xFF


Broadcast address:

0xFF


Broadcast packets are received by all nodes.

Broadcast communication is **one-way only** and does **not support ACK responses**.

This prevents multiple devices from transmitting acknowledgements at the same time and avoids bus collisions.


---

# Commands

The command byte is application-defined.

Examples:

READ_SENSOR
WRITE_OUTPUT
DEVICE_STATUS
CONFIGURATION


OwiBus does not restrict the meaning of commands, allowing flexible application design.


---

# Reliability

OwiBus provides:

## CRC8 verification

Each packet contains CRC8 calculated over:

- address fields
- sequence number
- flags
- command
- payload


## Sequence numbers

Sequence numbers allow:

- packet tracking
- ACK matching
- communication validation


## Optional ACK

Packets may request acknowledgement.

Example:

Node A

DATA + ACK_REQUEST
|
|
v

Node B

ACK_REPLY


This allows reliable communication when required without adding overhead to every packet.


---

# API

## Constructor

```cpp
OwiBus(uint8_t pin);

Creates a bus instance.

Example:

OwiBus bus(13);

begin()

void begin(uint8_t deviceId);

Starts communication and assigns device address.

Example:

bus.begin(5);

send()

bool send(
uint8_t receiver,
uint8_t command,
const char *text,
uint8_t flags
);

Send packet to another node.

Example:

bus.send(
2,
CMD_STATUS,
"OK",
ACK_REQ
);

update()

void update();

Must be called continuously in the main loop.

Example:

void loop()
{
    bus.update();
}

available()

bool available();

Returns true when a valid packet is received.
ackReceived()

bool ackReceived();

Returns true when an acknowledgement packet arrives.
sender()

uint8_t sender();

Returns sender address.
command()

uint8_t command();

Returns received command.
sequence()

uint8_t sequence();

Returns packet sequence number.
text()

const char* text();

Returns received payload.
Hardware

Designed for AVR microcontrollers.

Tested:

    Arduino Uno (ATmega328P)

    Arduino Mega 2560 (ATmega2560)

Communication requires:

    one GPIO data line

    common GND

    pull-up resistor recommended for longer cables

Relationship with OwiBusPHY

OwiBus requires OwiBusPHY.

Installation:

OwiBusPHY
    +
OwiBus

OwiBusPHY provides the physical communication.

OwiBus provides the network protocol.
Example Applications

Possible uses:

    sensor networks

    home automation

    automobile custom networks

    distributed controllers

    embedded device networks

    AVR multi-node systems

    custom industrial communication buses

Project Philosophy

OwiBus is not intended to replace high-speed communication standards.

Instead it provides a simple, lightweight and deterministic bus for embedded systems where:

    simplicity is important

    hardware resources are limited

    multiple devices must share one communication line

License

MIT License

Copyright (c) 2026 Ivaylo Panov
