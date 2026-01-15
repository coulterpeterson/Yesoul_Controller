/** Yesoul to Keyboard Controller
 *
 * Connects to Yesoul S3 bike (Fitness Machine Service), reads data,
 * and sends commands over Serial to a companion Python script to simulate key
 * presses.
 *
 * Protocol:
 * ">>> CMD"  -> Actionable command for Python script
 * "LOG: msg" -> Informational message for User display
 *
 * Logic:
 * - Cadence > 0  -> Hold 'W'
 * - Speed > 15km/h -> Hold 'B' (in addition to W)
 */
#include <Arduino.h>
#include <NimBLEDevice.h>

// Config
const float SPEED_THRESHOLD_KMH = 15.0;
const int MIN_CADENCE_RPM = 1; // Minimum cadence to trigger 'W'

// State tracking
bool w_key_active = false;
bool b_key_active = false;

// Bike connection details
static BLEUUID serviceUUID("1826"); // Fitness Machine
static BLEUUID charUUID("2ad2");    // Indoor Bike Data

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

// Helper to send formatted commands to the Python script
void sendCommand(const char *cmd) { Serial.printf(">>> %s\n", cmd); }

// Helper to send log messages to the Python script
void sendLog(const char *format, ...) {
  char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.printf("LOG: %s\n", buffer);
}

// Reset keys when disconnected
void releaseAllKeys() {
  if (w_key_active) {
    sendCommand("W_OFF");
    w_key_active = false;
  }
  if (b_key_active) {
    sendCommand("B_OFF");
    b_key_active = false;
  }
}

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                           uint8_t *pData, size_t length, bool isNotify) {

  // Expected format for Indoor Bike Data (0x2AD2):
  // Bytes 0-1: Flags
  // Bytes 2-3: Speed (uint16, 0.01 km/h)
  // Bytes 4-5: Cadence (uint16, 0.5 rpm)

  if (length < 6)
    return;

  uint16_t speedRaw = pData[2] | (pData[3] << 8);
  float speedKmh = speedRaw * 0.01;

  uint16_t cadenceRaw = pData[4] | (pData[5] << 8);
  int cadenceRpm = cadenceRaw / 2; // Scaled by 0.5

  // Send status update for user visibility
  sendLog("Speed: %.2f km/h | Cadence: %d rpm", speedKmh, cadenceRpm);

  // Logic for 'W' Key (Pedaling)
  if (cadenceRpm >= MIN_CADENCE_RPM) {
    if (!w_key_active) {
      sendCommand("W_ON");
      w_key_active = true;
    }
  } else {
    // If we stopped pedaling
    if (w_key_active) {
      sendCommand("W_OFF");
      w_key_active = false;
    }
  }

  // Logic for 'B' Key (Speeding)
  if (speedKmh > SPEED_THRESHOLD_KMH) {
    if (!b_key_active) {
      sendCommand("B_ON");
      b_key_active = true;
    }
  } else {
    if (b_key_active) {
      sendCommand("B_OFF");
      b_key_active = false;
    }
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) { sendLog("Connected to Bike"); }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    sendLog("Disconnected from Bike");
    releaseAllKeys(); // Safety release
  }
};

bool connectToServer() {
  sendLog("Forming a connection to %s",
          myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remote BLE Server.
  pClient->connect(myDevice);
  sendLog("Connected to server");

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    sendLog("Failed to find our service UUID: %s",
            serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    sendLog("Failed to find our characteristic UUID: %s",
            charUUID.toString().c_str());
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
    // sendLog("BLE Advertised Device found: %s",
    // advertisedDevice->toString().c_str());

    if (advertisedDevice->haveServiceUUID() &&
        advertisedDevice->isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = advertisedDevice;
      doConnect = true;
      doScan = true;
      sendLog("Found Yesoul Bike! Stopping scan and connecting...");
    }
  }
};

void setup() {
  Serial.begin(115200);
  sendLog("Starting Yesoul Keyboard Controller...");

  NimBLEDevice::init("");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  sendLog("Scanning for fitness machine...");
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      sendLog("We are now connected to the BLE Server.");
    } else {
      sendLog("We have failed to connect to the server.");
    }
    doConnect = false;
  }

  if (!connected && doScan) {
    // Retry scan if lost
    BLEDevice::getScan()->start(0);
  }

  delay(10);
}
