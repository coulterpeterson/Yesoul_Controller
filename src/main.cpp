/** Yesoul to Native USB Keyboard
 *
 * Connects to Yesoul S3 bike (Fitness Machine Service), reads data,
 * and sends NATIVE KEYSTROKES via the ESP32-S3 USB Port.
 *
 * HARDWARE REQUIREMENT:
 * You must plug the USB cable into the "USB" or "OTG" port of the ESP32-S3
 * for keystrokes to work. The "UART" port is only for debugging/uploading.
 */
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

USBHIDKeyboard Keyboard;

// Config
const float SPEED_THRESHOLD_KMH = 30.0;
const int MIN_CADENCE_RPM = 1;

// State tracking
bool w_key_active = false;
bool b_key_active = false;

static BLEUUID serviceUUID("1826"); // Fitness Machine
static BLEUUID charUUID("2ad2");    // Indoor Bike Data

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

// Helper to send logs to Serial (debugging)
void sendLog(const char *format, ...) {
  char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.printf("LOG: %s\n", buffer);
}

void releaseAllKeys() {
  if (w_key_active) {
    Keyboard.release('w');
    w_key_active = false;
    sendLog("Released W");
  }
  if (b_key_active) {
    Keyboard.release('b');
    b_key_active = false;
    sendLog("Released B");
  }
}

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                           uint8_t *pData, size_t length, bool isNotify) {
  if (length < 6)
    return;

  uint16_t speedRaw = pData[2] | (pData[3] << 8);
  float speedKmh = speedRaw * 0.01;

  uint16_t cadenceRaw = pData[4] | (pData[5] << 8);
  int cadenceRpm = cadenceRaw / 2;

  static unsigned long lastStatsLog = 0;
  if (millis() - lastStatsLog > 1000) {
    sendLog("Speed: %.2f km/h | Cadence: %d rpm", speedKmh, cadenceRpm);
    lastStatsLog = millis();
  }

  // --- KEYBOARD LOGIC ---

  // W Key (Pedaling)
  if (cadenceRpm >= MIN_CADENCE_RPM) {
    if (!w_key_active) {
      Keyboard.press('w');
      w_key_active = true;
      sendLog("Holding W");
    }
  } else {
    if (w_key_active) {
      Keyboard.release('w');
      w_key_active = false;
      sendLog("Released W");
    }
  }

  // B Key (Speeding)
  if (speedKmh > SPEED_THRESHOLD_KMH) {
    if (!b_key_active) {
      Keyboard.press('b');
      b_key_active = true;
      sendLog("Holding B");
    }
  } else {
    if (b_key_active) {
      Keyboard.release('b');
      b_key_active = false;
      sendLog("Released B");
    }
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) { sendLog("Connected to Bike"); }
  void onDisconnect(BLEClient *pclient) {
    connected = false;
    sendLog("Disconnected from Bike");
    releaseAllKeys();
  }
};

bool connectToServer() {
  sendLog("Forming a connection to %s",
          myDevice->getAddress().toString().c_str());
  BLEClient *pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);
  sendLog("Connected to server");

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice *advertisedDevice) {
    if (advertisedDevice->haveServiceUUID() &&
        advertisedDevice->isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      sendLog("FOUND TARGET BIKE! Connecting...");
    }
  }
};

void setup() {
  Serial.begin(115200);
  Keyboard.begin();
  USB.begin(); // Start Native USB stack

  delay(1000);
  sendLog("Starting Native USB Keyboard Mode...");

  NimBLEDevice::init("");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);

  pBLEScan->start(0, nullptr);
}

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      sendLog("Currently connected.");
    } else {
      BLEDevice::getScan()->start(0, nullptr);
    }
    doConnect = false;
  }

  if (!connected && !doConnect && !BLEDevice::getScan()->isScanning()) {
    BLEDevice::getScan()->start(0, nullptr);
  }

  // Heartbeat
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    if (!connected)
      sendLog("Status: Scanning... (Use OTG port for Keyboard)");
    lastLog = millis();
  }

  delay(100);
}
