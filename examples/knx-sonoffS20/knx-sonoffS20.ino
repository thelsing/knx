#include <knx.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif

#define RELAYPIN 12

// create named references for easy access to group objects
#define goSwitch knx.getGroupObject(1)
#define goBlock knx.getGroupObject(2)
#define goStatus knx.getGroupObject(3)


// callback from switch-GO
void switchCallback(GroupObject& go)
{
    if (goBlock.value())
        return;
    
    bool value = goSwitch.value();
    digitalWrite(RELAYPIN, value);
    goStatus.value(value);
}

void setup()
{
    Serial.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial;

#ifdef ARDUINO_ARCH_ESP8266
    WiFiManager wifiManager;    
    wifiManager.autoConnect("knx-sonoffS20");
#endif
	
    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    if (knx.configured())
    {
        // register callback for reset GO
        goSwitch.callback(switchCallback);
        goSwitch.dataPointType(Dpt(1, 1));
        goBlock.dataPointType(Dpt(1, 3));
        goStatus.dataPointType(Dpt(1, 2));
    }
    
    // start the framework. Will get wifi first.
    knx.start();
}

void loop() 
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if(!knx.configured())
        return;

    // nothing else to do.
}
