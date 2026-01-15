import serial
import serial.tools.list_ports
import pyautogui
import time
import sys

# Configuration
BAUD_RATE = 115200

def find_esp32_port():
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        if "CP210" in port.description or "CH340" in port.description or "SLAB_USBtoUART" in port.description or "USB Serial" in port.description:
            return port.device
    return None

def main():
    print("--- Yesoul Serial to Keyboard Bridge ---")
    
    port = None
    if len(sys.argv) > 1:
        port = sys.argv[1]
    else:
        print("Scanning for ESP32...")
        port = find_esp32_port()
    
    if not port:
        print("Error: Could not automatically find ESP32 port.")
        print("Please specify port manually: python serial_to_keyboard.py <COM_PORT>")
        print("Available ports:")
        for p in serial.tools.list_ports.comports():
            print(f"  - {p.device} ({p.description})")
        return

    print(f"Connecting to {port} at {BAUD_RATE} baud...")
    
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=0.1)
    except serial.SerialException as e:
        print(f"Failed to connect: {e}")
        return

    print("Connected! Listening for commands...")
    print("Press Ctrl+C to exit.")

    pyautogui.FAILSAFE = True

    try:
        while True:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
            except Exception:
                continue
            
            if not line:
                continue
                
            if line.startswith(">>>"):
                cmd = line.split(">>>")[1].strip()
                print(f"[ACTION] {cmd}")
                
                if cmd == "W_ON":
                    pyautogui.keyDown('w')
                elif cmd == "W_OFF":
                    pyautogui.keyUp('w')
                elif cmd == "B_ON":
                    pyautogui.keyDown('b')
                elif cmd == "B_OFF":
                    pyautogui.keyUp('b')
            
            elif line.startswith("LOG:"):
                msg = line.split("LOG:")[1].strip()
                # Print status updates, possibly overwriting line if you wanted advanced UI, 
                # but simple print is safer for cross-platform compatibility.
                print(f"[STATUS] {msg}")
            
            else:
                # Other debug info
                print(f"[DEBUG]  {line}")

    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        print("Releasing keys and closing connection.")
        pyautogui.keyUp('w')
        pyautogui.keyUp('b')
        ser.close()

if __name__ == "__main__":
    main()
