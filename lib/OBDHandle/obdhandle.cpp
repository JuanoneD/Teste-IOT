#include "obdhandle.h"

// Definição das variáveis estáticas
BLEUUID OBDHandle::serviceUUID;
BLEUUID OBDHandle::charUUID_TX;
BLEUUID OBDHandle::charUUID_RX;
BLERemoteCharacteristic* OBDHandle::pCharTX = nullptr;
BLERemoteCharacteristic* OBDHandle::pCharRX = nullptr;

BLEClient* OBDHandle::pClient = nullptr;
String OBDHandle::lastResponse = "";
bool OBDHandle::debugEnabled = false;
HardwareSerial* OBDHandle::debugSerial = nullptr;

void OBDHandle::debugPrint(String message) {
    if (debugEnabled && debugSerial != nullptr) {
        debugSerial->println("[OBDHandle] " + message);
    }
}

bool OBDHandle::begin() {
    BLEDevice::init("ESP32_PAINEL");
    pClient = BLEDevice::createClient();
    return  true;
}
void OBDHandle::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    for (int i = 0; i < length; i++) {
        lastResponse += (char)pData[i];
    }
    
    if(lastResponse.indexOf('>') != -1) {
        MessageHandle::processAndShowMessage(lastResponse);
        debugPrint("Message Received: " + lastResponse);
        lastResponse = "";
    }
}

void OBDHandle::sendCommand(String command) {
    if(!command.endsWith("\r")) command += "\r";
    
    debugPrint("Sending: " + command);

    lastResponse = ""; 
    pCharTX->writeValue((uint8_t*)command.c_str(), command.length(), false);

    unsigned long startTime = millis();
    unsigned long timeout = 1000; 

    if (command.startsWith("AT")) {
        timeout = 3000; 
    }
    
    while (lastResponse.indexOf('>') == -1) {
        if (millis() - startTime > timeout) {
            debugPrint("Command timeout after " + String(timeout) + "ms");
            return;
        }
        delay(10);
        yield(); 
    }

    debugPrint("Response received in " + String(millis() - startTime) + "ms");
}

void OBDHandle::sendStarterCommand(){
    OBDHandle::sendCommand("ATZ");    // Reset the chip
    OBDHandle::sendCommand("ATE0");   // Echo Off
    OBDHandle::sendCommand("ATH0");   // Headers Off
    OBDHandle::sendCommand("ATSP1");  // FORD STREET Protocol
    OBDHandle::sendCommand("ATAT1");  // Adaptive Timing
    OBDHandle::sendCommand("ATL0");   // Linefeeds Off
}

bool OBDHandle::connect(const char* address) {
    debugPrint("Attempting to connect to: " + String(address));
    
    BLEAddress targetAddress(address);

    if (pClient->connect(targetAddress)) {
        debugPrint("BLE connection successful");
        
        BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService != nullptr) {
            debugPrint("Service found");
            
            pCharTX = pRemoteService->getCharacteristic(charUUID_TX);
            pCharRX = pRemoteService->getCharacteristic(charUUID_RX);
            
            if (pCharTX != nullptr && pCharRX != nullptr) {
                debugPrint("Characteristics found");
                
                if(pCharRX->canNotify()) {
                    pCharRX->registerForNotify([](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
                        OBDHandle::notifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);
                    });
                }
                sendStarterCommand();
                debugPrint("Connection established successfully");
                return true;
            } else {
                debugPrint("ERROR: Characteristics not found");
            }
        } else {
            debugPrint("ERROR: Service not found");
        }
    } else {
        debugPrint("ERROR: BLE connection failed");
    }
    return false;
}

void OBDHandle::checkECU() {
    debugPrint("Checking ECU status...");
    sendCommand("0100\r");
}

void OBDHandle::setServiceUUID(const char* uuid) {
    serviceUUID = BLEUUID(uuid);
}

void OBDHandle::setCharUUID_TX(const char* uuid) {
    charUUID_TX = BLEUUID(uuid);
}

void OBDHandle::setCharUUID_RX(const char* uuid) {
    charUUID_RX = BLEUUID(uuid);
}

void OBDHandle::enableDebug(bool enable) {
    debugEnabled = enable;
}

void OBDHandle::setDebugSerial(HardwareSerial* serial) {
    debugSerial = serial;
}