#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../include/datadefinition.h"

// sensor address
static BLEAddress targetAddress("66:1e:32:7a:35:0e");
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEClient* pClient;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// UUIDs padrão para ELM327 BLE (v2.1 chinês)
static BLEUUID serviceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID_TX("0000fff2-0000-1000-8000-00805f9b34fb"); // Escrita
static BLEUUID charUUID_RX("0000fff1-0000-1000-8000-00805f9b34fb"); // Leitura
static BLERemoteCharacteristic* pCharTX;
static BLERemoteCharacteristic* pCharRX;

static int ledPin = 2;
bool ledActive = false;
unsigned long lastBlink = 0;
int messagesFromRPM = 0;

// Var
CONNECTION_STATUS status = CONNECTION_STATUS::DISCONNECTED;
ECU_STATUS ecu_state = ECU_STATUS::SLEEP;

String lastResponse = "";

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    for (int i = 0; i < length; i++) {
        lastResponse += (char)pData[i];
    }
}

String sendATCommand(String command) {
    if(status == CONNECTION_STATUS::DISCONNECTED) return "";
    if(!command.endsWith("\r")) command += "\r"; // Garante o carriage return no final

    lastResponse = ""; 
    pCharTX->writeValue((uint8_t*)command.c_str(), command.length(), false); // sending the message using pCharTX

    unsigned long startWait = millis();
    while(lastResponse == "" && (millis() - startWait < 800)) { 
      if (lastResponse.indexOf('>') != -1) {
        break; // Mensagem completa!
      }
      delay(1);
    }
    Serial.println("Response received in " + String(millis() - startWait) + " ms");
    String cleanRes = lastResponse;
    lastResponse = "";
    cleanRes.replace(">", "");
    cleanRes.trim();
    return cleanRes;
}

bool CheckECU() {
    String response = sendATCommand("0100\r");
    Serial.println(response);
    if (response.indexOf("41 00") != -1) {
        return true; // ECU ON
    }
    delay(1000);
    return false; // ECU OFF
}

void startConnectionWithOBD2() {
    BLEDevice::init("ESP32_PAINEL");
    pClient = BLEDevice::createClient();
    
    if (pClient->connect(targetAddress)) {
        BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService != nullptr) {
            pCharTX = pRemoteService->getCharacteristic(charUUID_TX);
            pCharRX = pRemoteService->getCharacteristic(charUUID_RX);
            
            if (pCharTX != nullptr && pCharRX != nullptr) {
              if(pCharRX->canNotify()) {
                    pCharRX->registerForNotify(notifyCallback); // connect the notification callback
              }
                Serial.println("RX / TX UP");
                status = CONNECTION_STATUS::CONNECTED;

                // Agora a luz deve piscar!
                sendATCommand("ATZ");    // Reseta o chip
                delay(1000);
                sendATCommand("ATE0");   // Echo Off (fundamental para a função processarDados funcionar)
                sendATCommand("ATH0");   // Headers Off (limpa a resposta deixando só os dados)
                sendATCommand("ATSP1");  // PROTOCOLO FORD STREET
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    lcd.init();
    delay(1000);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("-System Started-");
    Serial.println("--- System Started ---");

    startConnectionWithOBD2();
}

void updateLed(bool state) {
  digitalWrite(ledPin, state ? HIGH : LOW);
  ledActive = state;
}

void messageProcessRPM(String data) {
    data.replace(" ", "");
    data.replace(">", "");
    data.trim();

    int indexRPM = data.indexOf("410C");
    if (indexRPM != -1 && data.length() >= indexRPM + 8) {
        int A = strtol(data.substring(indexRPM + 4, indexRPM + 6).c_str(), NULL, 16);
        int B = strtol(data.substring(indexRPM + 6, indexRPM + 8).c_str(), NULL, 16);
        int rpm = ((A * 256) + B) / 4;
        if(rpm >= 2000) {
            updateLed(true);
        } else {
            updateLed(false);
        }
        Serial.printf(">>> RPM: %d\n", rpm);
        lcd.setCursor(0, 0);
        lcd.print("RPM: ");
        if(rpm < 1000) lcd.print(" ");
        lcd.print(rpm);
    }
}
void processarTemperatura(String data) {
    // A resposta do sensor para 0105 será algo como "41 05 5A"
    // 5A em Hex = 90 em Decimal. 90 - 40 = 50°C.
    
    data.replace(" ", ""); // Remove espaços
    int index = data.indexOf("4105");
    
    if (index != -1 && data.length() >= index + 6) {
        String hexVal = data.substring(index + 4, index + 6);
        int tempDecimal = strtol(hexVal.c_str(), NULL, 16);
        int tempFinal = tempDecimal - 40;

        // Exibe no LCD
        lcd.setCursor(0, 1);
        lcd.print("TEMP: ");
        lcd.print(tempFinal);
        lcd.write(223); // Caractere de grau (°)
        lcd.print("C   ");
        Serial.printf(">>> Temp: %d C\n", tempFinal);
    }
}

void loop() {
  if(status == CONNECTION_STATUS::DISCONNECTED)
  {
    if(millis() - lastBlink > 500) {
      updateLed(!ledActive);
      lastBlink = millis();
      Serial.println("Trying to connect with OBD...");
      lcd.setCursor(0, 0);
      lcd.print("Connecting OBD...");
      startConnectionWithOBD2();
    }
    return;
  }

  if(ecu_state == ECU_STATUS::SLEEP) {
    if(millis() - lastBlink > 100) {
      updateLed(!ledActive);
      lastBlink = millis();
      Serial.println("Connect with OBD, Wait ECU.");
      lcd.setCursor(0, 0);
      lcd.print("Connect with OBD");
      lcd.setCursor(0, 1);
      lcd.print(" Wait ECU...   ");
    }
    
    if(CheckECU()) {
      ecu_state = ECU_STATUS::AWAKE;
      updateLed(false); 
      Serial.println("ECU Started.");
      lcd.setCursor(0, 1);
      lcd.print(" ECU Started!");
      delay(1000);
      lcd.clear();
    }
  }
  else 
  {
    String rawRPM = sendATCommand("010C");

    if(rawRPM.indexOf("NO DATA") != -1 || rawRPM.indexOf("ERROR") != -1) {
        ecu_state = ECU_STATUS::SLEEP;
        Serial.println("ECU Connection is OFF.");
    } else {
        messageProcessRPM(rawRPM);
        messagesFromRPM++;
        if(messagesFromRPM >= 20) {
            delay(200);
            messagesFromRPM = 0;
            String res = sendATCommand("0105");
            processarTemperatura(res);
        }
    }


    delay(200); // Ciclo rápido de leitura
  }
}

