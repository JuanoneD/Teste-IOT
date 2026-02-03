#ifndef MESSAGEHANDLE_H
#define MESSAGEHANDLE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "../datadefinition.h"

class MessageHandle {
private:
    static LiquidCrystal_I2C *lcd;
    
    static bool debugEnabled;
    static HardwareSerial* debugSerial;
    static ECU_STATUS *ecu_state;

    static void processRPMMessage(String message);
    static void processTemperatureMessage(String message);
    static void processCheckECUMessage(String message);
    static void debugPrint(String message);
    
public:
    static void setECUState(ECU_STATUS* state);
    static void setLCD(LiquidCrystal_I2C *lcdInstance);
    static void processAndShowMessage(String message);
    static void enableDebug(bool enable);
    static void setDebugSerial(HardwareSerial* serial);
};

#endif