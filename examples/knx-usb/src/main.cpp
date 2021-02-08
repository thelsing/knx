#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <knx.h>

/*
 * USB stuff
*/
#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

#pragma message ("USB_VID=" STRINGIFY(USB_VID))
#pragma message ("USB_PID=" STRINGIFY(USB_PID))
#pragma message ("USB_MANUFACTURER=" STRINGIFY(USB_MANUFACTURER))
#pragma message ("USB_PRODUCT=" STRINGIFY(USB_PRODUCT))

Adafruit_USBD_HID usb_hid;

// Invoked when received SET_REPORT control request or
// received data on interrupt OUT endpoint
void setReportCallback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* data, uint16_t bufSize)
{
	// we don't use multiple report and report ID
	(void) report_id;
	(void) report_type;

    UsbTunnelInterface::receiveHidReport(data, bufSize);
}

bool sendHidReport(uint8_t* data, uint16_t length)
{
	// We do not use reportId of the TinyUSB sendReport()-API here but instead provide it in the first byte of the buffer
    return usb_hid.sendReport(0, data, length);
}

bool isSendHidReportPossible()
{
    return usb_hid.ready();
}

/*
 * KNX stuff
*/

// create macros easy access to group objects
#define goTemperature knx.getGroupObject(1)
#define goHumidity knx.getGroupObject(2)

uint32_t cyclSend = 0;
uint8_t sendCounter = 0;
long lastsend = 0;

/******************************************************************************************/

/*
 * setup()
*/
void setup(void)
{
    Serial1.begin(115200);
    ArduinoPlatform::SerialDebug = &Serial1;
    Serial1.println("Start.");

    usb_hid.enableOutEndpoint(true);
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(UsbTunnelInterface::getKnxHidReportDescriptor(), UsbTunnelInterface::getHidReportDescriptorLength());
    usb_hid.setReportCallback(NULL, setReportCallback);

    usb_hid.begin();

    // wait until device mounted
    while( !USBDevice.mounted() ) delay(1);

    println("KNX USB Interface enabled.");

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    if (knx.individualAddress() == 0)
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

/*
 * loop()
*/
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
