#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <Adafruit_NeoPixel.h>

#define LEFT_LED_PIN 2
#define RIGHT_LED_PIN 4
#define NUMPIXELS 1

static BLEUUID serviceUUID("12345678-1234-5678-1234-56789abcdef0");
static BLEUUID charUUID("abcdefab-cdef-1234-5678-1234567890ab");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEScan* pBLEScan;

Adafruit_NeoPixel leftPixels(NUMPIXELS, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightPixels(NUMPIXELS, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      pBLEScan->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
    }
  }
};

class ClientCallbacks: public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    connected = true;
    Serial.println("Connected to server");
  }

  void onDisconnect(BLEClient* pClient) {
    connected = false;
    Serial.println("Disconnected from server, restarting scan");
    doScan = true;
  }
};

void setup() {
  Serial.begin(115200);
  leftPixels.begin(); // Initialize the NeoPixel library
  leftPixels.show();  // Initialize all pixels to 'off'
  rightPixels.begin(); // Initialize the NeoPixel library
  rightPixels.show();  // Initialize all pixels to 'off'
  
  // Initialize BLE
  BLEDevice::init("Beacon2");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, false); // Start scanning indefinitely
}

bool connectToServer(BLEAdvertisedDevice* myDevice) {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallbacks()); // Set callbacks
  Serial.println(" - Created client");

  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  pRemoteCharacteristic->registerForNotify(notifyCallback);

  return true;
}

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    char buttonState = (char)pData[0];
    if (buttonState == '1') {
      leftPixels.setPixelColor(0, leftPixels.Color(255, 100, 0)); // Yellow color
      leftPixels.show();
    } else if (buttonState == '2') {
      rightPixels.setPixelColor(0, rightPixels.Color(255, 100, 0)); // Yellow color
      rightPixels.show();
    } else if (buttonState == '3') {
      leftPixels.setPixelColor(0, leftPixels.Color(255, 0, 0)); // Red color
      rightPixels.setPixelColor(0, rightPixels.Color(255, 0, 0)); // Red color
      leftPixels.show();
      rightPixels.show();
    } else {
      leftPixels.setPixelColor(0, leftPixels.Color(0, 0, 0)); // LED off
      rightPixels.setPixelColor(0, rightPixels.Color(0, 0, 0)); // LED off
      leftPixels.show();
      rightPixels.show();
    }
}

void loop() {
  if (doConnect) {
    if (connectToServer(myDevice)) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  if (doScan) {
    pBLEScan->start(0); // Restart scanning indefinitely
    doScan = false;
  }
}
