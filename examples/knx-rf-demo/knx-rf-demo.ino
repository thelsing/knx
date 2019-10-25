#include <knx.h>

// create macros easy access to group objects
#define goTemperature knx.getGroupObject(1)
#define goHumidity knx.getGroupObject(2)

uint32_t cyclSend = 0;
uint8_t sendCounter = 0;
long lastsend = 0;

// Entry point for the example
void setup(void)
{
    Serial1.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial1;
    delay(1000);
    Serial1.println("start");

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    if (knx.induvidualAddress() == 0)
      knx.progMode(true);


    if (knx.configured())
    {
        cyclSend = knx.paramInt(0);
        Serial1.print("Zykl. send:");
        Serial1.println(cyclSend);
        goTemperature.dataPointType(Dpt(9, 1));
        goHumidity.dataPointType(Dpt(9, 1));
    }

    // start the framework.
    knx.start();
}

// Function that is looped forever
void loop(void)
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if(!knx.configured())
        return;

    long now = millis();
    if ((now - lastsend) < 3000)
        return;

    lastsend = now;

    float temp = 1.2345;
    float humi = 60.2;
    String output = String(millis());
    output += ", " + String(temp);
    output += ", " + String(humi);
    Serial1.println(output);
        
    if (sendCounter++ == cyclSend)
    {
        sendCounter = 0;
    
        goTemperature.value(temp);
        goHumidity.value(humi);
    }

}
