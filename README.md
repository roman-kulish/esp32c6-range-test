# ESP32-C6 Wireless Performance Test Framework

This project provides a framework specifically designed for testing and comparing the **range, latency, and packet loss** performance of the **ESP32-C6** microcontroller using different wireless communication methods, including:

*   **ESP-NOW**
*   **Wi-Fi 4 (802.11n)**
*   **Wi-Fi 6 (802.11ax)**
*   **Wi-Fi LR (802.11b Long Range)**

## Features

*   **Sender & Receiver Roles:** Easily configure devices as packet senders or receivers.
*   **GPS Integration:** Utilizes GPS modules for:
    *   Accurate timestamping of packets.
    *   Adaptive system time synchronization using `adjtime` to maintain clock accuracy without abrupt jumps.
    *   Logging sender and receiver coordinates.
*   **Performance Logging:** Receivers log detailed packet information, including:
    *   Sequence numbers
    *   Sender and receiver timestamps (microseconds)
    *   Latency
    *   RSSI (Received Signal Strength Indicator)
    *   Packet loss rate calculation
    *   GPS coordinates and satellite info for both nodes
    *   Calculated distance between nodes
*   **Modular Design:** Easily adaptable to different communication protocols/modes by implementing the `Protocol` interface.

## Framework Note

This project utilizes the `pioarduino` framework via PlatformIO. This is currently necessary due to the lack of official Arduino framework support for the ESP32-C6 target within the standard Arduino ESP32 core.
