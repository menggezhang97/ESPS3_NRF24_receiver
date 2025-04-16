# ESP32-S3 + NRF24L01 Receiver

This is an Arduino sketch for **ESP32-S3** using an NRF24L01 module.

## Setup

- NRF24L01 is connected to **SPI**
- Channel: **110**
- Address: `"ABCDE"`
- Transmission mode: **No ACK, No interrupt** (simple & unsafe)

## Data Frame Format

The NRF24 sender transmits:

- **5 bytes header** + **27 bytes data**
- Each sensor sends its data in separate frames
- One "group" of data = sensor readings taken at the **same physical timestamp**

## Matched Sender

- The sender must follow the same NRF24 config (channel, address, payload structure).
- The data must be readed under same time stamp.
---

© 2025 menggezhang97
