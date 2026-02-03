#ifndef OBDHANDLE
#define OBDHANDLE

#include <Arduino.h>
#include <BLEDevice.h>
#include <messagehandle.h>

class OBDHandle {
private:

static BLEUUID serviceUUID;
static BLEUUID charUUID_TX;
static BLEUUID charUUID_RX;
static BLERemoteCharacteristic* pCharTX;
static BLERemoteCharacteristic* pCharRX;

static BLEClient* pClient;
static String lastResponse;
static bool debugEnabled;
static HardwareSerial* debugSerial;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
static void sendStarterCommand();
static void debugPrint(String message);


public:
static void setServiceUUID(const char* uuid);
static void setCharUUID_TX(const char* uuid);
static void setCharUUID_RX(const char* uuid);
static bool begin();
static bool connect(const char* address);
static void sendCommand(String command);
static void checkECU();
static void enableDebug(bool enable);
static void setDebugSerial(HardwareSerial* serial);

};

#endif