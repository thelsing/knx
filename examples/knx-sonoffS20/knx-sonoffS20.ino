#include <knx.h>

#define RELAYPIN 12

// declare array of all groupobjects with their sizes in byte
GroupObject groupObjects[]
{
    GroupObject(1),
    GroupObject(1),
    GroupObject(1)
};

// create named references for easy access to group objects
GroupObject& goSwitch = groupObjects[0];
GroupObject& goBlock = groupObjects[1];
GroupObject& goStatus = groupObjects[2];



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

    // register group objects
    knx.registerGroupObjects(groupObjects, 3);
    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // register callback for reset GO
    goSwitch.callback(switchCallback);

    // start the framework. Will get wifi first.
    knx.start();
}

void loop() 
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (!knx.configured())
        return;

    // nothing else to do.
}
