#include <knx.h>
#include <WiFiManager.h>

#define RELAYPIN 12

// create named references for easy access to group objects
#define goSwitch knx.getGroupObject(0)
#define goBlock knx.getGroupObject(1)
#define goStatus knx.getGroupObject(2)


// callback from switch-GO
void switchCallback(GroupObject& go)
{
    if (goBlock.objectReadBool())
        return;
    
    bool value = goSwitch.objectReadBool();
    digitalWrite(RELAYPIN, value);
    goStatus.objectWrite(value);
}

void setup()
{
    SerialDBG.begin(115200);

    WiFiManager wifiManager;    
    wifiManager.autoConnect("knx-sonoffS20");
	
    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    if (knx.configured())
    {
        // register callback for reset GO
        goSwitch.callback(switchCallback);
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
