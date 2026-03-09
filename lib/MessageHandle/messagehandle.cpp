#include "messagehandle.h"
#include <ctime>

LiquidCrystal_I2C* MessageHandle::lcd = nullptr;
bool MessageHandle::debugEnabled = false;
HardwareSerial* MessageHandle::debugSerial = nullptr;
ECU_STATUS* MessageHandle::ecu_state = nullptr;
unsigned long MessageHandle::lastEngineLoadRequestTime = 0;
int MessageHandle::lastRPMValue = 0;
unsigned long MessageHandle::lastSpeedRequestTime = 0;

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
        lastRPMValue = rpm;
    }
}

void MessageHandle::processTemperatureMessage(String message) {
    int index = message.indexOf("4105");
    
    if (index != -1 && message.length() >= index + 6) {
        String hexVal = message.substring(index + 4, index + 6);
        int tempDecimal = strtol(hexVal.c_str(), NULL, 16);
        int tempFinal = tempDecimal - 40;
        
        // Exibe no LCD
        lcd->setCursor(11, 1);
        if(tempFinal < 100) lcd->print(" ");
        if(tempFinal < 10) lcd->print(" ");
        lcd->print(tempFinal);
        lcd->write(223); // Caractere de grau (°)
        lcd->print("C");
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
        case ENGINE_LOAD_MUX:
            processEngineLoadMessage(clearMessage);
            break;
        case SPEED_MUX:
            processSpeedMessage(clearMessage);
            break;
        default:
        break;
    }
}

void MessageHandle::processSpeedMessage(String message) {
    int index = message.indexOf("410D");
    unsigned long currentTime = millis();
    
    if (index != -1 && message.length() >= index + 6) {
        int speedKmh = strtol(message.substring(index + 4, index + 6).c_str(), NULL, 16);

        lcd->setCursor(0, 1);
        if(speedKmh < 100) lcd->print(" ");
        if(speedKmh < 10) lcd->print(" ");
        lcd->print(speedKmh);
        lcd->print("km/h");
        debugPrint(">>> Speed: " + String(speedKmh) + " km/h\n");

        if (lastSpeedRequestTime > 0) {
            double deltaTime = (currentTime - lastSpeedRequestTime) / 1000.0;
            double metersTraveled = (speedKmh / 3.6) * deltaTime;
            
            float totalKm = PreferencesHandle::getInstance().getDistanceTraveled() + (metersTraveled / 1000.0);
            PreferencesHandle::getInstance().setDistanceTraveled(totalKm);
        }
        lastSpeedRequestTime = currentTime;
    }
}

void MessageHandle::processEngineLoadMessage(String message) {
    int index = message.indexOf("4104");
    unsigned long currentTime = millis();

    if(lastEngineLoadRequestTime == 0) {
        lastEngineLoadRequestTime = currentTime;
        return;
    }
    
    if (index != -1 && message.length() >= index + 6) {
        String hexVal = message.substring(index + 4, index + 6);
        int loadDecimal = strtol(hexVal.c_str(), NULL, 16);
        int loadFinal = (loadDecimal * 100) / 255;
        float deltaTime = (currentTime - lastEngineLoadRequestTime)/1000.0; 

        float fuelConsumption = (lastRPMValue * loadFinal * (PreferencesHandle::getInstance().getConsumptionFactor())) * deltaTime;
        PreferencesHandle::getInstance().setFuel(PreferencesHandle::getInstance().getFuel() - fuelConsumption);

        float tripfuelConsumed = PreferencesHandle::getInstance().getTripFuelUsed() + fuelConsumption;
        PreferencesHandle::getInstance().setTripFuelUsed(tripfuelConsumed);

        lastEngineLoadRequestTime = currentTime;
        debugPrint(">>> Engine Load: " + String(loadFinal) + " %\n");
        lcd->setCursor(13, 0);
        lcd->print(int(PreferencesHandle::getInstance().getFuel() * 100 / PreferencesHandle::getInstance().getTankCapacity()));
        lcd->print("%");
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