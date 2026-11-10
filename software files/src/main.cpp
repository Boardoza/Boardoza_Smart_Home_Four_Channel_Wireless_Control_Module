#include <Arduino.h>
#include <Boardoza_FT810.h>
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>

#include "ui_assets.h"

// ESP32-S3 -> FT810 GPIO connections
constexpr uint8_t FT810_CS_PIN = 7;
constexpr uint8_t FT810_PWR_PDN_PIN = 38;
constexpr uint8_t FT810_INT_PIN = 37;
constexpr uint8_t FT810_SCK_PIN = 17;
constexpr uint8_t FT810_MISO_PIN = 16;
constexpr uint8_t FT810_MOSI_PIN = 15;

// TFT touch TAG values
constexpr uint8_t TAG_RELAY_1 = 1;
constexpr uint8_t TAG_RELAY_2 = 2;
constexpr uint8_t TAG_RELAY_3 = 3;
constexpr uint8_t TAG_RELAY_4 = 4;
constexpr uint8_t TAG_CONNECT = 10;

// BLE identifiers that must exactly match the ESP32-H2 receiver
constexpr char TARGET_DEVICE_NAME[] = "Boardoza Project";
constexpr char SERVICE_UUID[] = "12345678-1234-1234-1234-123456789ABC";
constexpr char RELAY_UUID_1[] = "00000001-1234-1234-1234-123456789ABC";
constexpr char RELAY_UUID_2[] = "00000002-1234-1234-1234-123456789ABC";
constexpr char RELAY_UUID_3[] = "00000003-1234-1234-1234-123456789ABC";
constexpr char RELAY_UUID_4[] = "00000004-1234-1234-1234-123456789ABC";

Boardoza_FT810 ft810(
    FT810_INT_PIN,
    FT810_PWR_PDN_PIN,
    FT810_CS_PIN,
    FT810_SCK_PIN,
    FT810_MISO_PIN,
    FT810_MOSI_PIN);

NimBLEClient* bleClient = nullptr;
NimBLERemoteCharacteristic* relayCharacteristics[4] = {nullptr, nullptr, nullptr, nullptr};

bool ft810Ready = false;
bool uiAssetsReady = false;
bool bleConnected = false;
bool bleScanning = false;
bool relayStates[4] = {false, false, false, false};
volatile bool disconnectedEvent = false;
uint8_t previousTouchTag = 0;

char titleText[] = "BOARDOZA CONTROL";
char subtitleText[] = "SMART HOME CONTROL";
char connectText[24] = "CONNECT TO DEVICE";
char connectionStatusText[40] = "DEVICE: DISCONNECTED";
char relayButtonText[4][32];
char relayChannelText[4][24] = {
    "CONTROL CHANNEL 01",
    "CONTROL CHANNEL 02",
    "CONTROL CHANNEL 03",
    "CONTROL CHANNEL 04",
};
char relayOffLegendText[] = "RED: OFF";
char relayOnLegendText[] = "GREEN: ON";
char relaySectionText[] = "DEVICE CONTROLS";
char emptyButtonText[] = "";

class BoardozaClientCallbacks : public NimBLEClientCallbacks {
 public:
  void onConnect(NimBLEClient* client) override {
    Serial.println("[BLE] ESP32-H2 connection established");
  }

  void onDisconnect(NimBLEClient* client, int reason) override {
    Serial.printf("[BLE] Connection closed. Reason: %d\n", reason);
    disconnectedEvent = true;
  }
};

BoardozaClientCallbacks clientCallbacks;

void clearRemoteCharacteristics() {
  for (uint8_t i = 0; i < 4; i++) {
    relayCharacteristics[i] = nullptr;
  }
}

void setConnectionUi(bool connected) {
  bleConnected = connected;

  if (connected) {
    snprintf(connectText, sizeof(connectText), "END CONNECTION");
    snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: CONNECTED");
  } else {
    snprintf(connectText, sizeof(connectText), "CONNECT TO DEVICE");
    snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: DISCONNECTED");
  }
}

void updateRelayButtonTexts() {
  snprintf(relayButtonText[0], sizeof(relayButtonText[0]), "AIR CONDITIONER");
  snprintf(relayButtonText[1], sizeof(relayButtonText[1]), "LIGHTING");
  snprintf(relayButtonText[2], sizeof(relayButtonText[2]), "SMART CURTAIN");
  snprintf(relayButtonText[3], sizeof(relayButtonText[3]), "MUSIC PLAYER");
}

void loadUiAssets() {
  Serial.println("[UI] Loading Boardoza logo and device icons into FT810 memory");
  ft810.loadBitmap(
      UI_LOGO_ASSET.address,
      UI_LOGO_ASSET.pixels,
      UI_LOGO_ASSET.byteCount);

  for (uint8_t index = 0; index < 4; index++) {
    ft810.loadBitmap(
        UI_RELAY_ASSETS[index].address,
        UI_RELAY_ASSETS[index].pixels,
        UI_RELAY_ASSETS[index].byteCount);
  }

  uiAssetsReady = true;
  Serial.println("[UI] Visual assets are ready");
}

void drawRelayCard(uint8_t relayIndex, uint16_t x, uint16_t y) {
  const bool active = relayStates[relayIndex];
  const uint32_t borderColor = active ? 0x4ADE80 : 0xFF6B6B;
  const uint32_t pressColor = active ? 0x4ADE80 : 0xFF7A7A;
  const uint32_t releaseColor = active ? 0x16A34A : 0xC62828;
  const uint32_t gradientColor = active ? 0x064E3B : 0x7F1D1D;
  const uint32_t hintColor = active ? 0xDCFCE7 : 0xFFE4E6;

  // Shadow and thin outline separate relay cards from the background.
  ft810.drawRectangle(x + 5, y + 6, 360, 130, 0x05070C);
  ft810.drawRectangle(x - 2, y - 2, 364, 134, borderColor);

  ft810.add_button(
      pressColor,
      releaseColor,
      gradientColor,
      WHITE,
      relayIndex + 1,
      x,
      y,
      360,
      130,
      30,
      emptyButtonText);

  if (uiAssetsReady) {
    ft810.drawBitmap(
        UI_RELAY_ASSETS[relayIndex].address,
        UI_RELAY_ASSETS[relayIndex].width,
        UI_RELAY_ASSETS[relayIndex].height,
        x + 18,
        y + 17,
        relayIndex + 1);
  }

  ft810.drawText(
      x + 132,
      y + 18,
      20,
      hintColor,
      relayChannelText[relayIndex],
      relayIndex + 1);
  ft810.drawText(
      x + 132,
      y + 58,
      29,
      WHITE,
      relayButtonText[relayIndex],
      relayIndex + 1);
}

void drawControllerScreen() {
  if (!ft810Ready) {
    return;
  }

  updateRelayButtonTexts();

  ft810.waitForCommandCompletion();
  ft810.startFrame();
  ft810.clearScreencolorBackground(9, 14, 25);

  // Header card. Red/green are reserved for relay states only.
  const uint32_t connectionColor = bleConnected ? 0x22C55E : 0xEF4444;
  ft810.drawRectangle(0, 0, 800, 112, 0x121A2B);

  if (uiAssetsReady) {
    ft810.drawBitmap(
        UI_LOGO_ASSET.address,
        UI_LOGO_ASSET.width,
        UI_LOGO_ASSET.height,
        24,
        15);
  } else {
    ft810.drawText(28, 16, 31, WHITE, titleText);
  }
  ft810.drawText(30, 75, 20, 0xAAB2BD, subtitleText);
  //ft810.drawText(350, 32, 18, 0xFF8A8A, relayOffLegendText);
  //ft810.drawText(350, 59, 18, 0x86EFAC, relayOnLegendText);

  // Connection status appears only beneath the connection button.
  // This keeps it separate from the heading and button label.
  ft810.drawRectangle(598, 12, 180, 60, 0x334155);
  ft810.add_button(
      0x64748B,
      0x334155,
      0x0F172A,
      0xE2E8F0,
      TAG_CONNECT,
      600,
      14,
      176,
      56,
      bleConnected ? 18 : 20,
      connectText);
  ft810.drawText(610, 82, 20, connectionColor, connectionStatusText);

  ft810.drawText(28, 124, 21, 0x94A3B8, relaySectionText);
  ft810.drawLine(28, 145, 744, 0, 0x334155, 8);

  drawRelayCard(0, 25, 160);
  drawRelayCard(1, 415, 160);
  drawRelayCard(2, 25, 300);
  drawRelayCard(3, 415, 300);

  ft810.endFrame();
}

void disconnectFromReceiver() {
  Serial.println("[BLE] Connection is being ended by the user");

  if (bleClient != nullptr && bleClient->isConnected()) {
    bleClient->disconnect();
  }

  clearRemoteCharacteristics();
  setConnectionUi(false);

  for (uint8_t i = 0; i < 4; i++) {
    relayStates[i] = false;
  }
}

bool connectToReceiver() {
  if (bleClient != nullptr && bleClient->isConnected()) {
    setConnectionUi(true);
    return true;
  }

  bleScanning = true;
  snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: SCANNING...");
  Serial.println("[BLE] Scanning for Boardoza Project (8 seconds)");
  drawControllerScreen();

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(100);
  scan->clearResults();

  NimBLEScanResults results = scan->getResults(8000, false);
  const NimBLEAdvertisedDevice* targetDevice = nullptr;
  const NimBLEUUID targetService(SERVICE_UUID);

  Serial.printf("[BLE] %d devices found during scan\n", results.getCount());

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice* device = results.getDevice(i);
    const bool serviceMatches = device->isAdvertisingService(targetService);
    const bool nameMatches = device->haveName() && device->getName() == TARGET_DEVICE_NAME;

    Serial.printf(
        "[BLE] Device %d: %s | name=%s | service=%s\n",
        i + 1,
        device->getAddress().toString().c_str(),
        device->haveName() ? device->getName().c_str() : "unnamed",
        serviceMatches ? "MATCHED" : "none");

    if (serviceMatches || nameMatches) {
      targetDevice = device;
      Serial.printf("[BLE] Target found: %s\n", device->toString().c_str());
      break;
    }
  }

  if (targetDevice == nullptr) {
    Serial.println("[BLE] ERROR: Boardoza Project was not found");
    snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: NOT FOUND");
    bleScanning = false;
    return false;
  }

  if (bleClient != nullptr) {
    if (bleClient->isConnected()) {
      bleClient->disconnect();
    }
    NimBLEDevice::deleteClient(bleClient);
    bleClient = nullptr;
  }

  bleClient = NimBLEDevice::createClient();
  if (bleClient == nullptr) {
    Serial.println("[BLE] ERROR: Could not create BLE client");
    snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: CLIENT ERROR");
    bleScanning = false;
    return false;
  }

  bleClient->setClientCallbacks(&clientCallbacks, false);
  bleClient->setConnectTimeout(5000);

  Serial.println("[BLE] Connecting to ESP32-H2");
  if (!bleClient->connect(targetDevice)) {
    Serial.println("[BLE] ERROR: Connection could not be established");
    snprintf(connectionStatusText, sizeof(connectionStatusText), "DEVICE: CONNECTION ERROR");
    NimBLEDevice::deleteClient(bleClient);
    bleClient = nullptr;
    bleScanning = false;
    return false;
  }

  Serial.println("[BLE] Looking for H2 GATT relay service");
  NimBLERemoteService* service = bleClient->getService(SERVICE_UUID);
  if (service == nullptr) {
    const int errorCode = bleClient->getLastError();
    Serial.printf(
        "[BLE] ERROR: Target service not found. Code: %d (%s)\n",
        errorCode,
        NimBLEUtils::returnCodeToString(errorCode));
    bleClient->disconnect();
    clearRemoteCharacteristics();
    setConnectionUi(false);
    bleScanning = false;
    return false;
  }

  relayCharacteristics[0] = service->getCharacteristic(RELAY_UUID_1);
  relayCharacteristics[1] = service->getCharacteristic(RELAY_UUID_2);
  relayCharacteristics[2] = service->getCharacteristic(RELAY_UUID_3);
  relayCharacteristics[3] = service->getCharacteristic(RELAY_UUID_4);

  for (uint8_t i = 0; i < 4; i++) {
    if (relayCharacteristics[i] == nullptr || !relayCharacteristics[i]->canWrite()) {
      Serial.printf("[BLE] ERROR: Relay %u characteristic is missing or not writable\n", i + 1);
      bleClient->disconnect();
      clearRemoteCharacteristics();
      setConnectionUi(false);
      bleScanning = false;
      return false;
    }
  }

  Serial.println("[BLE] All four relay characteristics are ready");
  bleScanning = false;
  setConnectionUi(true);
  Serial.printf("[BLE] CONNECTED. RSSI: %d dBm\n", bleClient->getRssi());
  return true;
}

bool sendRelayCommand(uint8_t relayIndex, bool newState) {
  if (!bleConnected || bleClient == nullptr || !bleClient->isConnected()) {
    Serial.println("[BLE] Command not sent: ESP32-H2 is not connected");
    setConnectionUi(false);
    return false;
  }

  if (relayIndex >= 4 || relayCharacteristics[relayIndex] == nullptr) {
    Serial.println("[BLE] Command not sent: invalid characteristic");
    return false;
  }

  const std::string value(1, newState ? '1' : '0');
  const bool success = relayCharacteristics[relayIndex]->writeValue(value, true);

  Serial.printf(
      "[BLE] Relay %u -> %u, result: %s\n",
      relayIndex + 1,
      newState ? 1 : 0,
      success ? "SUCCESS" : "ERROR");

  return success;
}

void handleTouch(uint8_t tag) {
  if (tag == TAG_CONNECT) {
    if (bleConnected) {
      disconnectFromReceiver();
    } else if (!bleScanning) {
      connectToReceiver();
    }
    return;
  }

  if (tag >= TAG_RELAY_1 && tag <= TAG_RELAY_4) {
    const uint8_t relayIndex = tag - 1;
    const bool newState = !relayStates[relayIndex];

    if (sendRelayCommand(relayIndex, newState)) {
      relayStates[relayIndex] = newState;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("BOARDOZA TFT + BLE CONTROLLER STARTING");
  Serial.println("FT810 INT=GPIO37, PWR/PDN=GPIO38");
  Serial.println("========================================");

  ft810Ready = ft810.begin();
  if (!ft810Ready) {
    Serial.println("[FT810] Initialization failed; BLE interface will not start");
    return;
  }

  Serial.println("[FT810] TFT and resistive touch are ready");
  loadUiAssets();

  NimBLEDevice::init("Boardoza El Tableti");
  NimBLEDevice::setPower(3);
  Serial.println("[BLE] Client ready; tap CONNECT TO DEVICE on the screen");

  setConnectionUi(false);
  drawControllerScreen();
}

void loop() {
  if (!ft810Ready) {
    delay(1000);
    return;
  }

  if (disconnectedEvent) {
    disconnectedEvent = false;
    clearRemoteCharacteristics();
    setConnectionUi(false);
    for (uint8_t i = 0; i < 4; i++) {
      relayStates[i] = false;
    }
  }

  const uint8_t currentTouchTag = static_cast<uint8_t>(ft810.touch_tag());

  // Respond to each touch only once.
  if (currentTouchTag != 0 && currentTouchTag != previousTouchTag) {
    Serial.printf("[TOUCH] TAG=%u\n", currentTouchTag);
    handleTouch(currentTouchTag);
  }

  previousTouchTag = currentTouchTag;
  drawControllerScreen();
  delay(25);
}
