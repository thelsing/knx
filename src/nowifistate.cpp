#include <ESP8266WiFi.h>

#include "nowifistate.h"
#include "wpsstate.h"
#include "runningstate.h"

NoWifiState noWifiState = NoWifiState();

void NoWifiState::shortButtonPress()
{
    switchToSate(wpsState);
}

void NoWifiState::longButtonPress()
{
    switchToSate(wpsState);
}

void NoWifiState::enterState()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin("", "");
    while (WiFi.status() == WL_DISCONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        Serial.printf("\nConnected successful to SSID '%s'\n", WiFi.SSID().c_str());
        switchToSate(runningState);
    }
}