#include "arch_config.h"

#ifdef USE_STATES
#include <ESP8266WiFi.h>

#include "wpsstate.h"
#include "runningstate.h"
#include "nowifistate.h"

WpsState wpsState = WpsState();

void WpsState::enterState()
{
    //invalidate old wifi settings first
    WiFi.begin("fobar", "a12");
    Serial.println("WPS config start");
    bool wpsSuccess = WiFi.beginWPSConfig();
    if (wpsSuccess) {
        String newSSID = WiFi.SSID();
        if (newSSID.length() > 0)
        {
            Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
            switchToSate(runningState);
        }
        else
        {
            Serial.printf("WPS failed.");
            switchToSate(noWifiState);
        }
    }
}

#endif
