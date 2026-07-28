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

OwiBusPHY provides the physical communication.

OwiBus provides the network protocol.

Example Applications

Possible uses:

    sensor networks

    home automation

    custom networks

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
