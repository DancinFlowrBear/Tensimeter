#include "BLEDevice.h"

static BLEUUID serviceUUID("0000FFF0-0000-1000-8000-00805F9B34FB");  // Primary Service UUID
static BLEUUID charUUID("0000FFF4-0000-1000-8000-00805F9B34FB");      // Characteristic UUID
static BLEUUID descriptorUUID("00002902-0000-1000-8000-00805F9B34FB"); // Descriptor UUID (for enabling notifications)
static BLEAddress sphygAddress("94:e3:6d:aa:6f:f3");                 // Your device's BLE address

static BLEClient* pClient;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static boolean isConnected = false;

// 🔹 Callback function when new data is received
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    // Only process if data starts with AA 0D
    if (pData[0] == 0xAA && pData[1] == 0x0D) {
        // Debugging the bytes involved in the systolic, diastolic, and BPM values
        int sys = pData[5];  // systolic value in decimal
        int dia = pData[6];  // diastolic value in decimal
        int bpm = pData[8];  // BPM value in decimal

        // Send only the systolic, diastolic, and BPM to ESP8266 over Serial1
        char dataToSend[64];
        snprintf(dataToSend, sizeof(dataToSend), "%d %d %d", sys, dia, bpm);
        Serial1.println(dataToSend);  // Send the parsed data to ESP8266

        // Debugging output for serial monitor (not sent to ESP8266)
        Serial.print("Parsed data sent to ESP8266: ");
        Serial.print(sys);
        Serial.print(" ");
        Serial.print(dia);
        Serial.print(" ");
        Serial.println(bpm);
    }
}

bool connectToDevice(BLEAddress pAddress) {
    Serial.println("🔗 Connecting to device...");
    pClient = BLEDevice::createClient();
    
    if (pClient->connect(pAddress)) {
        Serial.println("✅ Connected to sphygmomanometer!");

        BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr) {
            Serial.println("❌ Service not found!");
            return false;
        }

        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            Serial.println("❌ Characteristic not found!");
            return false;
        }

        Serial.println("🔔 Subscribing to notifications...");
        pRemoteCharacteristic->registerForNotify(notifyCallback);

        // Enable notifications by writing to the descriptor
        BLERemoteDescriptor* pDescriptor = pRemoteCharacteristic->getDescriptor(descriptorUUID);
        if (pDescriptor != nullptr) {
            uint8_t notificationOn[] = {0x01, 0x00};  // Enable notifications
            pDescriptor->writeValue(notificationOn, 2);
            Serial.println("✅ Notifications enabled!");
        } else {
            Serial.println("⚠️ Descriptor not found! Notifications might not work.");
        }

        isConnected = true;
        return true;
    }
    
    Serial.println("❌ Connection failed!");
    return false;
}

void setup() {
    Serial.begin(9600);
    Serial.println("🚀 Starting BLE Client...");
    
    BLEDevice::init("");

    if (connectToDevice(sphygAddress)) {
        Serial.println("✅ ESP32 connected to sphygmomanometer!");
    } else {
        Serial.println("❌ Failed to connect.");
    }

    // Start communication with ESP8266 over UART
    Serial1.begin(9600, SERIAL_8N1, 16, 17);  // Use appropriate RX/TX pins for ESP32-to-ESP8266
}

void loop() {
    if (!isConnected) {
        Serial.println("🔄 Reconnecting...");
        connectToDevice(sphygAddress);
    }

    delay(5000);  // Delay to avoid constant checking
}

