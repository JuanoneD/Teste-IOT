#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "../lib/datadefinition.h"
#include <obdhandle.h>
#include <messagehandle.h>

// sensor address
static String targetAddress = "66:1e:32:7a:35:0e";

// UUIDs
static String serviceUUID = "0000fff0-0000-1000-8000-00805f9b34fb";
static String charUUID_TX = "0000fff2-0000-1000-8000-00805f9b34fb"; // Escrita
static String charUUID_RX = "0000fff1-0000-1000-8000-00805f9b34fb"; // Leitura

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// cycle count
int messagesFromRPM = 0;

// Var
CONNECTION_STATUS status = CONNECTION_STATUS::DISCONNECTED;
ECU_STATUS ecu_state = ECU_STATUS::SLEEP;

void setup() {
    Serial.begin(115200);
    Wire.setClock(400000); // 400kHz I2C
    lcd.init();
    delay(1000);
    lcd.backlight();

    lcd.setCursor(0, 0);
    Serial.println("--- System Started ---");
    lcd.setCursor(0, 0);
    lcd.print("-System Started-");
 
    // Enable debug for OBDHandle
    OBDHandle::setDebugSerial(&Serial);
    OBDHandle::enableDebug(true);

    OBDHandle::setServiceUUID(serviceUUID.c_str());
    OBDHandle::setCharUUID_TX(charUUID_TX.c_str());
    OBDHandle::setCharUUID_RX(charUUID_RX.c_str());
    OBDHandle::begin();
    
    //MessageHandle::enableDebug(true);
    //MessageHandle::setDebugSerial(&Serial);

    MessageHandle::setLCD(&lcd);
    MessageHandle::setECUState(&ecu_state);
}

void loop() {
  if(status == CONNECTION_STATUS::DISCONNECTED)
  {
    Serial.println("Trying to connect with OBD...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting OBD...");
    
    if(OBDHandle::connect(targetAddress.c_str()))
    {
      status = CONNECTION_STATUS::CONNECTED;
      lcd.clear();
    }
    
    return;
  }

  if(ecu_state == ECU_STATUS::SLEEP) {
    Serial.println("Connect with OBD, Wait ECU.");
    lcd.setCursor(0, 0);
    lcd.print("Connect with OBD");
    lcd.setCursor(0, 1);
    lcd.print(" Wait ECU...   ");
    while (ecu_state == ECU_STATUS::SLEEP) {
      delay(500);
      OBDHandle::checkECU();
    
    }
    lcd.setCursor(0, 1);
    lcd.print("   ECU Awake!   ");
    delay(1000);
    lcd.clear();
  }

  if(messagesFromRPM >= 20) {
    messagesFromRPM = 0;
    OBDHandle::sendCommand("0105"); // Request Engine Coolant Temperature
  }else{
    OBDHandle::sendCommand("010C"); // Request RPM
    messagesFromRPM++;
  }
  delay(300);
}

