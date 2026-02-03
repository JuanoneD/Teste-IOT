#include "messagehandle.h"
#include <ctime>

LiquidCrystal_I2C* MessageHandle::lcd = nullptr;
bool MessageHandle::debugEnabled = false;
HardwareSerial* MessageHandle::debugSerial = nullptr;
ECU_STATUS* MessageHandle::ecu_state = nullptr;

void MessageHandle::processRPMMessage(String message) {
    int indexRPM = message.indexOf("410C");
    if (indexRPM != -1 && message.length() >= indexRPM + 8 && lcd != nullptr) {
        int A = strtol(message.substring(indexRPM + 4, indexRPM + 6).c_str(), NULL, 16);
        int B = strtol(message.substring(indexRPM + 6, indexRPM + 8).c_str(), NULL, 16);
        int rpm = ((A * 256) + B) / 4;
        lcd->setCursor(0, 0);
        debugPrint("Displaying RPM: " + String(rpm));
        lcd->print("RPM: ");
        if(rpm < 1000) lcd->print(" ");
        lcd->print(rpm);
    }
}

void MessageHandle::processTemperatureMessage(String message) {
    int index = message.indexOf("4105");
    
    if (index != -1 && message.length() >= index + 6) {
        String hexVal = message.substring(index + 4, index + 6);
        int tempDecimal = strtol(hexVal.c_str(), NULL, 16);
        int tempFinal = tempDecimal - 40;
        
        // Exibe no LCD
        lcd->setCursor(0, 1);
        lcd->print("TEMP: ");
        lcd->print(tempFinal);
        lcd->write(223); // Caractere de grau (°)
        lcd->print("C   ");
        debugPrint(">>> Temp: " + String(tempFinal) + " C\n");
    }
}

void MessageHandle::processCheckECUMessage(String message) {
    if(ecu_state == nullptr) return;
    *ecu_state = ECU_STATUS::AWAKE;
    debugPrint("ECU is AWAKE.");
}

void MessageHandle::processAndShowMessage(String message) {
    
    if(message.indexOf("NO DATA") != -1 || message.indexOf("ERROR") != -1) {
        debugPrint("ECU Connection is OFF.");
        if(ecu_state != nullptr) {
            *ecu_state = ECU_STATUS::SLEEP;
        }
        return;
    }
    
    int mux = strtol(message.substring(4, 6).c_str(), NULL, 16);

    debugPrint("Processing message with MUX: " + String(mux));

    String clearMessage = message;
    clearMessage.replace(" ", "");
    clearMessage.replace(">", "");
    clearMessage.trim();
    
    switch (mux) {
        case RPM_MUX:
            processRPMMessage(clearMessage);
            break;
        case TEMP_MUX:
            processTemperatureMessage(clearMessage);
            break;
        case CHECK_ECU_MUX:
            processCheckECUMessage(clearMessage);
            break;
        default:
        break;
    }
}

void MessageHandle::enableDebug(bool enable) {
    debugEnabled = enable;
}

void MessageHandle::setDebugSerial(HardwareSerial* serial) {
    debugSerial = serial;
}

void MessageHandle::debugPrint(String message) {
    if (debugEnabled && debugSerial != nullptr) {
        debugSerial->println("[MessageHandle] " + message);
    }
}

void MessageHandle::setECUState(ECU_STATUS* state) {
    ecu_state = state;
}

void MessageHandle::setLCD(LiquidCrystal_I2C* lcdInstance) {
    lcd = lcdInstance;
}