/*

This example defines three datapoints.
The first two are TEMPL type datapoints and have their own callback.
When no specific callback is attached to a datapoint, it uses the global callback.

Note the difference in return value between the callbacks:
for tempCallback uses value.getFloat() as TEMPL datapoints return a float.
globalCallback uses value.getString(char*,size_t). This method is independent of the returned type.

*/
#include <VitoWiFi.h>
#ifdef ARDUINO_ARCH_ESP8266
#define SerialDBG Serial1
#define SerialVito Serial
#endif


#ifdef ARDUINO_ARCH_SAMD
#define SerialDBG SerialUSB
#define SerialVito Serial1
namespace std {
    void __throw_bad_alloc()
    {
        SerialDBG.println("Unable to allocate memory");
    }

    void __throw_length_error(char const*e)
    {
        SerialDBG.print("Length Error :");
        SerialDBG.println(e);
    }
}
#endif

VitoWiFi_setProtocol(P300);

DPTemp outsideTemp("outsideTemp", "boiler", 0x5525);
DPTemp boilerTemp("boilertemp", "boiler", 0x0810);
DPStat pumpStat("pump", "heating1", 0x2906);

void tempCallbackHandler(const IDatapoint& dp, DPValue value) {
    SerialDBG.print(dp.getGroup());
    SerialDBG.print(" - ");
    SerialDBG.print(dp.getName());
    SerialDBG.print(": ");
    SerialDBG.println(value.getFloat());
}

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
    SerialDBG.print(dp.getGroup());
    SerialDBG.print(" - ");
    SerialDBG.print(dp.getName());
    SerialDBG.print(" is ");
    char value_str[15] = { 0 };
    value.getString(value_str, sizeof(value_str));
    SerialDBG.println(value_str);
}

void setup() {
    SerialDBG.begin(115200);
    delay(5000);
    SerialDBG.println("start");
    
    outsideTemp.setCallback(tempCallbackHandler);
    boilerTemp.setCallback(tempCallbackHandler);
    
    // this callback will be used for all DPs without specific callback
    // must be set after adding at least 1 datapoint
    VitoWiFi.setGlobalCallback(globalCallbackHandler);    
                                                        
    VitoWiFi.setup(&SerialVito);

    SerialDBG.println(F("Setup finished..."));
}

void loop() {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis > 60 * 1000UL) {
        // read all values every 60 seconds
  lastMillis = millis();
        VitoWiFi.readAll();
    }
    VitoWiFi.loop();
}
