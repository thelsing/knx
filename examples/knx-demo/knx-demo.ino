#include <Arduino.h>

#define USE_KNX

#ifdef USE_KNX
    #include <knx.h>

    #if (MASK_VERSION != 0x07B0) && (defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_ESP32)
        #include <WiFiManager.h>
    #endif

#if USE_W5X00 == 1
    #include <SPI.h>
    #include <Ethernet.h>
#endif

    // create named references for easy access to group objects
    #define goCurrent knx.getGroupObject(1)
    #define goMax knx.getGroupObject(2)
    #define goMin knx.getGroupObject(3)
    #define goReset knx.getGroupObject(4)

    float currentValue = 0;
    float maxValue = 0;
    float minValue = RAND_MAX;
    long lastsend = 0;

    void measureTemp()
    {
        long now = millis();
        if ((now - lastsend) < 2000)
            return;

        lastsend = now;
        int r = rand();
        currentValue = (r * 1.0) / (RAND_MAX * 1.0);
        currentValue *= 100 * 100;

        // write new value to groupobject
        goCurrent.value(currentValue);

        if (currentValue > maxValue)
        {
            maxValue = currentValue;
            goMax.value(maxValue);
        }

        if (currentValue < minValue)
        {
            minValue = currentValue;
            goMin.value(minValue);
        }
    }

    // callback from reset-GO
    void resetCallback(GroupObject& go)
    {
        if (go.value())
        {
            maxValue = 0;
            minValue = 10000;
        }
    }
#endif

void setup()
{
    Serial.begin(115200);
    while (!Serial) {
        delay(1000); // wait for serial port to connect. Needed for native USB port only, long for vscode stupid text
    }

    // IP stuff
    #if USE_W5X00 == 1
        println("****** Set SS to pin 4 ******");
        Ethernet.init(4);
        byte _ma[6] = {0xC0, 0xFF, 0xEE, 0xC0, 0xDE, 0x00};
        println("****** Bring up ethernet connection ******");
        Ethernet.begin(_ma);
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            println("****** No Ethernet shield found ******.");
            println("****** Ideling forever... ******");
            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware or link
            }
        }
        if (Ethernet.hardwareStatus() != EthernetW5500) {
            println("****** Currently only working on W5500 controller ******");
            println("****** Ideling forever... ******");
            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware or link
            }
        }
        if (Ethernet.linkStatus() == LinkOFF) {
            println("Link status: Off (No network cable connected or port on switch down)");
            println("****** Ideling forever... ******");
            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware or link
            }
        }
        println("****** Ethernet shield connected ******");
        println("****** W5500 Ethernet controller detected ******");
        println("****** Link status: On ******");
        char ar[50];
        sprintf(ar, "****** Mac address used: %02X:%02X:%02X:%02X:%02X:%02X ******", _ma[0], _ma[1], _ma[2], _ma[3], _ma[4], _ma[5]);
        println(ar);
        println("****** Ethernet connection up ******");
        println("****** Connected with DHCP ******");
        IPAddress _ip = Ethernet.localIP();
        sprintf(ar, "****** IP address: %d.%d.%d.%d ******", _ip[0], _ip[1], _ip[2], _ip[3]);
        println(ar);
        _ip = Ethernet.subnetMask();
        sprintf(ar, "****** Subnet mask: %d.%d.%d.%d ******", _ip[0], _ip[1], _ip[2], _ip[3]);
        println(ar);
        _ip = Ethernet.gatewayIP();
        sprintf(ar, "****** Gateway: %d.%d.%d.%d ******", _ip[0], _ip[1], _ip[2], _ip[3]);
        println(ar);
    #endif

    #ifdef USE_KNX
        ArduinoPlatform::SerialDebug = &Serial;
        println("****** Serial debug running ******");

        randomSeed(millis());

        #if ((MASK_VERSION != 0x07B0) && (defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_ESP32))
            WiFiManager wifiManager;
            wifiManager.autoConnect("knx-demo");
        #endif

        println("****** Start reading configuration ******");
        // read adress table, association table, groupobject table and parameters from eeprom
        knx.readMemory();
        
        // print values of parameters if device is already configured
        if (knx.configured())
        {
            println("****** Configuration found ******");
            // register callback for reset GO
            goReset.callback(resetCallback);
            goReset.dataPointType(DPT_Trigger);
            goCurrent.dataPointType(DPT_Value_Temp);
            goMin.dataPointType(DPT_Value_Temp);
            goMax.dataPointType(DPT_Value_Temp);

            Serial.print("Timeout: ");
            Serial.println(knx.paramByte(0));
            Serial.print("Zykl. senden: ");
            Serial.println(knx.paramByte(1));
            Serial.print("Min/Max senden: ");
            Serial.println(knx.paramByte(2));
            Serial.print("Aenderung senden: ");
            Serial.println(knx.paramByte(3));
            Serial.print("Abgleich: ");
            Serial.println(knx.paramByte(4));
        }
        else{
            println("****** No stored configuration found ******");
        }

        // pin or GPIO the programming led is connected to. Default is LED_BUILTIN
        // knx.ledPin(LED_BUILTIN);
        // is the led active on HIGH or low? Default is LOW
        // knx.ledPinActiveOn(HIGH);
        // pin or GPIO programming button is connected to. Default is 0
        println("****** Set KNX reset button to pin 1 ******");
        knx.buttonPin(1);
        println("****** Starting knx framework ******");
        // start the framework.
        knx.start();
        println("****** Knx framework started ******");
    #endif
}

void loop()
{
#ifdef USE_KNX
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (!knx.configured())
        return;

    // measureTemp();
#endif

}
