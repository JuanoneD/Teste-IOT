    #include "htmlinterface.h"
    
    String HTMLInterface::getHTML() {
    float meanTrip = (PreferencesHandle::getInstance().getTripFuelUsed() > 0.001) ? (PreferencesHandle::getInstance().getDistanceTraveled() / PreferencesHandle::getInstance().getTripFuelUsed()) : 0.0;

    String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: -apple-system, sans-serif; background: #1c1c1e; color: white; text-align: center; padding: 20px; }";
    html += ".card { background: #2c2c2e; padding: 20px; border-radius: 20px; margin-bottom: 20px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); }";
    html += ".litros { font-size: 48px; font-weight: bold; color: #30d158; }";
    html += "button { width: 100%; padding: 15px; margin: 10px 0; border: none; border-radius: 12px; font-size: 18px; font-weight: bold; cursor: pointer; }";
    html += ".btn-reset { background: #ff3b30; color: white; }"; // Vermelho
    html += ".btn-add { background: #007aff; color: white; }";   // Azul
    html += ".btn-save { background: #34c759; color: white; }";  // Verde
    html += "input { width: 100%; padding: 12px; border-radius: 8px; border: 1px solid #3a3a3c; background: #1c1c1e; color: white; margin-top: 10px; font-size: 16px; }";
    html += "</style></head><body>";

    html += "<h1>Fiesta Street</h1>";
    
    html += "<div class='card'>";
    html += "<div>Combustível Estimado</div>";
    html += "<div class='litros'>" + String(PreferencesHandle::getInstance().getFuel(), 1) + "L</div>";
    html += "<div>" + String((PreferencesHandle::getInstance().getFuel() / PreferencesHandle::getInstance().getTankCapacity()) * 100, 0) + "% do tanque</div>";
    html += "</div>";

    html += "<form action='/reset' method='POST'><button class='btn-reset'>RESET (TANQUE CHEIO)</button></form>";
    html += "<form action='/add10' method='POST'><button class='btn-add'>ADICIONAR 10 LITROS</button></form>";

    html += "<div class='card'>";
    html += "<h3>Calibrar Consumo</h3>";
    html += "<p style='font-size: 13px; color: #8e8e93; margin-bottom: 10px;'>Quanto entrou de gasolina no posto?</p>";
    html += "<form action='/factorCalibration' method='POST'>";
    html += "<input type='number' step='0.01' name='liters_supplied' placeholder='Ex: 4.40' required>";
    html += "<button class='btn-save'>CALCULAR E ATUALIZAR</button></form>";
    html += "<div style='margin-top: 15px; font-size: 11px; color: #666;'>Fator atual: " + String(PreferencesHandle::getInstance().getConsumptionFactor(), 12) + "</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<h3>Trip (Viagem Atual)</h3>";
    html += "<div style='color: #007aff;'>Distância: " + String(PreferencesHandle::getInstance().getDistanceTraveled(), 2) + " km</div>";
    html += "<div style='color: #30d158;'>Gasto: " + String(PreferencesHandle::getInstance().getTripFuelUsed(), 3) + " L</div>";
    html += "<div style='font-size: 24px; margin-top:10px;'>Média: " + String(meanTrip, 1) + " km/L</div>";

    // Botão de Reset agora zera os DOIS
    html += "<form action='/resetTrip' method='POST'><button class='btn-add' style='background:#5856d6'>ZERAR TRIP</button></form>";
    html += "</div>";

    html += "</body></html>";
    return html;
    }

    void HTMLInterface::handleRoot() {
    server.send(200, "text/html", getHTML());
    }

    void HTMLInterface::handleReset() {
    PreferencesHandle::getInstance().setFuel(PreferencesHandle::getInstance().getTankCapacity());
    server.sendHeader("Location", "/");
    server.send(303);
    }

    void HTMLInterface::handleAdd10() {
    PreferencesHandle::getInstance().setFuel(PreferencesHandle::getInstance().getFuel() + 10);
    if (PreferencesHandle::getInstance().getFuel() > PreferencesHandle::getInstance().getTankCapacity()) {
        PreferencesHandle::getInstance().setFuel(PreferencesHandle::getInstance().getTankCapacity());
    }
    server.sendHeader("Location", "/");
    server.send(303);
    }

    void HTMLInterface::handleFactorCalibration() {
    if (server.hasArg("liters_supplied")) {
        float realLitersSupplied = server.arg("liters_supplied").toFloat();
        
        float fullTank = PreferencesHandle::getInstance().getTankCapacity();
        float currentTank = PreferencesHandle::getInstance().getFuel();
        float virtualTankUsage = fullTank - currentTank;

        if (virtualTankUsage > 0.1 && realLitersSupplied > 0) {
            float oldFactor = PreferencesHandle::getInstance().getConsumptionFactor();
            
            float newFactor = oldFactor * (realLitersSupplied / virtualTankUsage);

            PreferencesHandle::getInstance().setConsumptionFactor(newFactor);
            PreferencesHandle::getInstance().setFuel(fullTank);
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void HTMLInterface::handleResetTrip() {
    PreferencesHandle::getInstance().setDistanceTraveled(0.0);
    PreferencesHandle::getInstance().setTripFuelUsed(0.0); // Zera o contador de consumo da viagem
    server.sendHeader("Location", "/");
    server.send(303);
}

    void HTMLInterface::begin() {
    WiFi.softAP("ESP32_PAINEL");
    server.on("/", std::bind(&HTMLInterface::handleRoot, this));
    server.on("/reset", HTTP_POST, std::bind(&HTMLInterface::handleReset, this));
    server.on("/add10", HTTP_POST, std::bind(&HTMLInterface::handleAdd10, this));
    server.on("/factorCalibration", HTTP_POST, std::bind(&HTMLInterface::handleFactorCalibration, this));
    server.on("/resetTrip", HTTP_POST, std::bind(&HTMLInterface::handleResetTrip, this));
    server.begin();
    }

    void HTMLInterface::handleClient() {
    server.handleClient();
    }

    HTMLInterface::HTMLInterface() {
    }

