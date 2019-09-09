#include <HDC100X.h>
#include <knx.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif

HDC100X HDC1(0x43);

// create macros easy access to group objects
#define goTemperature knx.getGroupObject(1)
#define goHumidity knx.getGroupObject(2)

long lastsend = 0;

// Helper functions declarations
void errLeds(void);

uint8_t sendCounter = 0;
uint32_t cyclSend = 0;

// Entry point for the example
void setup(void)
{
    Serial.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial;
    delay(5000);
    Serial.println("start");

#ifdef ARDUINO_ARCH_ESP8266
	WiFiManager wifiManager;    
    wifiManager.autoConnect("knx-hdc1008");
#endif

    // Programming LED on digital pin D5
    knx.ledPin(5);
    // Programming button on digital pin D7
    knx.buttonPin(7);    

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    HDC1.begin(HDC100X_TEMP_HUMI,HDC100X_14BIT,HDC100X_14BIT,DISABLE);
    
    if (knx.configured())
    {
		
        cyclSend = knx.paramInt(0);
        Serial.print("Zykl. send:");
        Serial.println(cyclSend);
    }
    
    // start the framework.
    knx.start();

    String output = "Timestamp [ms], temperature [°C], relative humidity [%]";
    Serial.println(output);
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

    float temp = HDC1.getTemp();
    float humi = HDC1.getHumi();
    String output = String(millis());
    output += ", " + String(temp);
    output += ", " + String(humi);
    Serial.println(output);
        
    if (sendCounter++ == cyclSend)
    {
        sendCounter = 0;
    
        goTemperature.value(temp);
        goHumidity.value(humi);
    }
}

void errLeds(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
