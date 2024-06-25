#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BUTTON_PIN_1 15  // GPIO pin for button 1
#define BUTTON_PIN_2 16  // GPIO pin for button 2
#define BUTTON_PIN_3 17  // GPIO pin for button 3

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;
bool buttonPressed_1 = false;
bool buttonPressed_2 = false;
bool buttonPressed_3 = false;

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcdefab-cdef-1234-5678-1234567890ab"

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  
  // Initialize BLE
  BLEDevice::init("Beacon1");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

void loop() {
  bool currentButtonState_1 = (digitalRead(BUTTON_PIN_1) == LOW);
  bool currentButtonState_2 = (digitalRead(BUTTON_PIN_2) == LOW);
  bool currentButtonState_3 = (digitalRead(BUTTON_PIN_3) == LOW);
  
  if (currentButtonState_1 != buttonPressed_1) {
    buttonPressed_1 = currentButtonState_1;
    Serial.println(buttonPressed_1 ? "Button 1 Pressed" : "Button 1 Released");
    pCharacteristic->setValue(buttonPressed_1 ? "1" : "0");
    pCharacteristic->notify();
  }
  if (currentButtonState_2 != buttonPressed_2) {
    buttonPressed_2 = currentButtonState_2;
    Serial.println(buttonPressed_2 ? "Button 2 Pressed" : "Button 2 Released");
    pCharacteristic->setValue(buttonPressed_2 ? "2" : "0");
    pCharacteristic->notify();
  }
  if (currentButtonState_3 != buttonPressed_3) {
    buttonPressed_3 = currentButtonState_3;
    Serial.println(buttonPressed_3 ? "Button 3 Pressed" : "Button 3 Released");
    pCharacteristic->setValue(buttonPressed_3 ? "3" : "0");
    pCharacteristic->notify();
  }
  delay(100);  // Adding a small delay to prevent flooding notifications
}
