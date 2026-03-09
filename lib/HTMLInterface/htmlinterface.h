#ifndef HTMLINTERFACE_H
#define HTMLINTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "../datadefinition.h"
#include <preferenceshandle.h>


class HTMLInterface {
public:
    HTMLInterface();
    void begin();
    void handleClient();

private:
    WebServer server;

    String getHTML();
    void handleRoot();
    void handleReset();
    void handleAdd10();
    void handleFactorCalibration();
    void handleResetTrip();
};

#endif