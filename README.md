# Yesoul Keyboard Controller

This project allows you to use a Yesoul S3 (or hopefully G1M Plus) spin bike as a game controller. It intercepts the bike's Bluetooth data using an ESP32 and simulates keyboard presses on your computer via USB. Shoutout to https://github.com/Raelx/Yesoul_BLE for the original codebase.

**How it works:**
1.  **Pedaling**: Holds down `W`.
2.  **Pedaling Fast (> 15 km/h)**: Holds down `B` (Boost) + `W`.

## Prerequisites

### Hardware
*   **Yesoul Bike** (or compatible FTMS Bluetooth bike).
*   **ESP32 Development Board** (e.g., ESP32-WROOM, NodeMCU-32S).
*   **USB C Cable** (data capable) to connect ESP32 to computer.

### Software
*   **VSCode** with **PlatformIO** extension (for flashing the ESP32).
*   **Python 3.x** (for running the bridge script on your computer).

---

## Setup Guide

### 1. Flash the ESP32
The ESP32 firmware handles the Bluetooth connection to the bike.

1.  Open this folder in VSCode.
2.  Ensure the **PlatformIO** extension is installed. It will automatically install the required libraries (`NimBLE-Arduino`) defined in `platformio.ini`.
3.  Connect your ESP32 to your Mac.
4.  Click the **PlatformIO Alien Icon** (sidebar) -> **Project Tasks** -> **env:esp32dev** -> **General** -> **Upload**.
5.  Wait for the "SUCCESS" message.

### 2. Setup Python Bridge
The Python script listens to the ESP32 over USB and presses keys.

#### Mac / Linux
It's recommended to use a virtual environment to avoid system package errors.

1.  Open a terminal in this folder.
2.  Create a virtual environment:
    ```bash
    python3 -m venv venv
    ```
3.  Activate it:
    ```bash
    source venv/bin/activate
    ```
4.  Install dependencies:
    ```bash
    pip install -r requirements.txt
    ```

#### Windows
1.  Open PowerShell or Command Prompt in this folder.
2.  Create a virtual environment:
    ```bash
    python -m venv venv
    ```
3.  Activate it:
    ```bash
    .\venv\Scripts\activate
    ```
4.  Install dependencies:
    ```bash
    pip install -r requirements.txt
    ```

### 3. Usage

1.  **Wake up the bike**: Start pedaling to wake up the Yesoul bike's Bluetooth.
2.  **Connect ESP32**: Plug the ESP32 into your computer.
3.  **Run the Script**:
    ```bash
    # (Ensure venv is active)
    python serial_to_keyboard.py
    ```
    *The script will attempt to auto-detect the ESP32 port.*

4.  **Pairing**:
    -   The script output will show: `[STATUS] Scanning for fitness machine...`
    -   When it finds the bike, it will say: `[STATUS] Found Yesoul Bike! ... Connected`.
    -   **Note**: If it doesn't find it, ensure your phone is NOT connected to the bike. The bike can only connect to one device at a time.

5.  **Ride**:
    -   Pedal to trigger `W`.
    -   Pedal faster (>15kph) to trigger `B`.
    -   You will see real-time stats in the terminal: `[STATUS] Speed: 20.5 km/h | Cadence: 80 rpm`.

---

## Troubleshooting

-   **"error: externally-managed-environment"**: Use the virtual environment steps above (step 2).
-   **Script can't find ESP32**:
    -   Find the port manually: `ls /dev/tty.*` (look for `SLAB_USBtoUART` or `usbserial`).
    -   Run specific port: `python serial_to_keyboard.py /dev/tty.SLAB_USBtoUART`
-   **Keys not pressing**:
    -   Mac requires **Accessibility Permissions** for the terminal to control the keyboard.
    -   Go to **System Settings** -> **Privacy & Security** -> **Accessibility**.
    -   Allow **Terminal** (or VSCode/iTerm).
-   **Bike not connecting**:
    -   Restart ESP32 (Press 'EN' button).
    -   Unplug bike power for 10s.
    -   Turn off Bluetooth on your phone/tablet.
