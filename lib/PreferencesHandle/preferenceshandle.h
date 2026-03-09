#ifndef PREFERENCES_HANDLE_H
#define PREFERENCES_HANDLE_H

#include <Preferences.h>
#include "../datadefinition.h"

class PreferencesHandle {
public:
    static PreferencesHandle& getInstance();
    float getFuel();
    void setFuel(float fuel);
    float getTankCapacity();
    void setTankCapacity(float capacity);
    float getConsumptionFactor();
    void setConsumptionFactor(float factor);
    float getDistanceTraveled();
    void setDistanceTraveled(float distance);
    float getTripFuelUsed();
    void setTripFuelUsed(float fuel);

private:
    static PreferencesHandle *instance;
    PreferencesHandle();
    Preferences prefs;

    float fuel;
    float tankCapacity;
    float consumptionFactor;
    float distanceTraveled;
    float tripFuelUsed;
    void savePreferences();
};

#endif
