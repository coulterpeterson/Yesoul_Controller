# Yesoul Keyboard Controller (Native USB)

This project turns your ESP32-S3 into a wireless bridge: it connects to your Yesoul Bike via Bluetooth and acts as a **physical USB Keyboard** for your computer. No Python scripts or drivers needed!

**How it works:**
1.  **Pedaling**: Holds down `W`.
2.  **Pedaling Fast (> 15 km/h)**: Holds down `B` (Boost) + `W`.

## Prerequisites

*   **ESP32-S3 Board** (Must have "USB" or "OTG" port support).
*   **Yesoul Bike** (or FTMS bike).

---

## Setup Guide

### 1. Flash the Firmware
1.  Connect ESP32-S3 via the **UART / COM** port (this is for programming).
2.  Open this folder in VSCode + PlatformIO.
3.  Upload using `env:esp32-s3-devkitc-1`.

### 2. Connect as Keyboard
1.  **Unplug** the cable from the UART port.
2.  **Plug** the cable into the **USB / OTG** port (this enables Keyboard mode).
    *   *Note: On some boards "USB" handles both, but usually there are two ports.*
3.  Your computer will detect a new "Keyboard".

### 3. Usage
1.  **Ride**: Pedal the bike.
2.  **Play**: Open any game, text editor, or Zwift. The keys `W` and `B` will be pressed automatically!

---

## Debugging
If you want to see logs:
1.  Keep the **UART** cable connected to view logs in PlatformIO Serial Monitor.
2.  Connect a **second** cable to the **USB/OTG** port for keyboard function.
