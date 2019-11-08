#include "usb_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "address_table_object.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a < b) ? (a) : (b))

Adafruit_USBD_HID usb_hid;

// HID report descriptor using TinyUSB's template
// Generic In Out with 64 bytes report (max)
static uint8_t const desc_hid_report[] =
{
  //TUD_HID_REPORT_DESC_GENERIC_INOUT(64)
 0x06, 0xA0, 0xFF, // Usage Page (Vendor Defined 0xFFA0)
0x09, 0x01,        // Usage (0x01)
0xA1, 0x01,        // Collection (Application)
0x09, 0x01,        //   Usage (0x01)
0xA1, 0x00,        //   Collection (Physical)
0x06, 0xA1, 0xFF,  //     Usage Page (Vendor Defined 0xFFA1)
0x09, 0x03,        //     Usage (0x03)
0x09, 0x04,        //     Usage (0x04)
0x15, 0x80,        //     Logical Minimum (-128)
0x25, 0x7F,        //     Logical Maximum (127)
0x35, 0x00,        //     Physical Minimum (0)
0x45, 0xFF,        //     Physical Maximum (-1)
0x75, 0x08,        //     Report Size (8)
0x85, 0x01,        //     Report ID (1)
0x95, 0x3F,        //     Report Count (63)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x05,        //     Usage (0x05)
0x09, 0x06,        //     Usage (0x06)
0x15, 0x80,        //     Logical Minimum (-128)
0x25, 0x7F,        //     Logical Maximum (127)
0x35, 0x00,        //     Physical Minimum (0)
0x45, 0xFF,        //     Physical Maximum (-1)
0x75, 0x08,        //     Report Size (8)
0x85, 0x01,        //     Report ID (1)
0x95, 0x3F,        //     Report Count (63)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0xC0               // End Collection  
};

struct _rx_queue_frame_t
{
    uint8_t* data;
    uint16_t length;
    _rx_queue_frame_t* next;
};

static struct _rx_queue_t
{
    _rx_queue_frame_t* front = NULL;
    _rx_queue_frame_t* back = NULL;
} _rx_queue;

static void addFrameRxQueue(CemiFrame& frame)
{
    _rx_queue_frame_t* rx_frame = new _rx_queue_frame_t;

    rx_frame->length = frame.totalLenght();
    rx_frame->data = new uint8_t[rx_frame->length];
    rx_frame->next = NULL;

/*
    print("cEMI USB RX len: ");
    print(rx_frame->length);

    printHex(" data:", rx_frame->data, rx_frame->length);
*/
    if (_rx_queue.back == NULL)
    {
        _rx_queue.front = _rx_queue.back = rx_frame;
    }
    else
    {
        _rx_queue.back->next = rx_frame;
        _rx_queue.back = rx_frame;
    }
}

static bool isRxQueueEmpty()
{
    if (_rx_queue.front == NULL)
    {
        return true;
    }
    return false;
}

static void loadNextRxFrame(uint8_t** receiveBuffer, uint16_t* receiveBufferLength)
{
    if (_rx_queue.front == NULL)
    {
        return;
    }
    _rx_queue_frame_t* rx_frame = _rx_queue.front;
    *receiveBuffer = rx_frame->data;
    *receiveBufferLength = rx_frame->length;
    _rx_queue.front = rx_frame->next;

    if (_rx_queue.front == NULL)
    {
        _rx_queue.back = NULL;
    }
    delete rx_frame;
}

static uint16_t parseReport(uint8_t* data, uint16_t dataLength)
{
	int8_t respDataSize = 0;
	bool forceSend = false;
	uint16_t length = 0;

	popWord(length, &data[5]);
	length = MIN(dataLength, length);

	Serial1.print("RX HID report: len: ");
	Serial1.println(length, DEC);

	for (int i = 0; i < (length + data[4] + 3); i++)
	{
		if (data[i] < 16)
			Serial1.print("0");
		Serial1.print(data[i], HEX);
		Serial1.print(" ");
	}
	Serial1.println("");

	if (	data[0] == 0x01 && // ReportID (fixed 0x01)
			data[1] == 0x13 && // PacketInfo must be 0x13 (SeqNo: 1, Type: 3)
			data[3] == 0x00 && // Protocol version (fixed 0x00)
			data[4] == 0x08 && // USB KNX Transfer Protocol Header Length (fixed 0x08)
			data[7] == 0x0F )  // Bus Access Server Feature (0x0F)
	{
		uint8_t serviceId = data[8];
		switch (serviceId)
		{
			case 0x01: // Device Feature Get
			{
				data[8] = 0x02;          // Device Feature Response

				uint8_t featureId = data[11];
				switch (featureId)
				{
					case 0x01: // Supported EMI types
						Serial1.println("Device Feature Get: Supported EMI types");
						respDataSize = 2;
						data[12] = 0x00; // USB KNX Transfer Protocol Body: Feature Data
						data[13] = 0x04; // USB KNX Transfer Protocol Body: Feature Data -> only cEMI supported
						break;
					case 0x02: // Host Device Descriptor Type 0
						Serial1.println("Device Feature Get: Host Device Descriptor Type 0");
						respDataSize = 2;
						data[12] = 0x00; // USB KNX Transfer Protocol Body: Feature Data
						data[13] = 0x00; // USB KNX Transfer Protocol Body: Feature Data
						break;
					case 0x03: // Bus connection status
						Serial1.println("Device Feature Get: Bus connection status");
						respDataSize = 1;
						data[12] = 1; // USB KNX Transfer Protocol Body: Feature Data
						break;
					case 0x04: // KNX manufacturer code
						Serial1.println("Device Feature Get: KNX manufacturer code");
						respDataSize = 2;
						data[12] = 0x00; // USB KNX Transfer Protocol Body: Feature Data
						data[13] = 0x00; // USB KNX Transfer Protocol Body: Feature Data -> Manufacturer Code
						break;
					case 0x05: // Active EMI type
						Serial1.println("Device Feature Get: Active EMI type");
						respDataSize = 1;
						data[12] = 0x03; // USB KNX Transfer Protocol Body: Feature Data -> cEMI ID
						break;
					default:
						respDataSize = 0;
						break;
				}
				break;
			}
			case 0x03: // Device Feature Set
			{
				uint8_t featureId = data[11];
				switch (featureId)
				{
					case 0x05: // Active EMI type
						Serial1.print("Device Feature Set: Active EMI type: ");
						if (data[12] < 16)
							Serial1.print("0");
						Serial1.println(data[12], HEX); // USB KNX Transfer Protocol Body: Feature Data -> EMI TYPE ID
						break;
					// All other featureIds must not be set
					case 0x01: // Supported EMI types
					case 0x02: // Host Device Descriptor Type 0
					case 0x03: // Bus connection status
					case 0x04: // KNX manufacturer code
					default:
						break;
				}
				break;
			}

			// These are only sent from the device to the host
			case 0x02: // Device Feature Response
			case 0x04: // Device Feature Info
			case 0xEF: // reserved, not used
			case 0xFF: // reserved (ESCAPE for future extensions)
			default:
				break;
		}
	}
	else if (	data[0] == 0x01 && // ReportID (fixed 0x01)
			data[1] == 0x13 && // PacketInfo must be 0x13 (SeqNo: 1, Type: 3)
			data[3] == 0x00 && // Protocol version (fixed 0x00)
			data[4] == 0x08 && // USB KNX Transfer Protocol Header Length (fixed 0x08)
			data[7] == 0x01 && // KNX Tunneling (0x01)
			data[8] == 0x03 )  // EMI ID: 3 -> cEMI
	{
		CemiFrame frame(&data[11], data[6]);
		addFrameRxQueue(frame);

		uint8_t messageCode = data[11];		
		switch(messageCode)
		{
			case 0xFC: // M_PropRead.req
			{
				data[11] = 0xFB; // M_PropRead.con
				if (data[15] == 0x34)
				{
					data[18] = 00; // PID_COMM_MODE: 0: Data Link Layer
				}
				else
				{
					data[16] = 0; // Number of elements must be 0 if negative response
					data[18] = 7; // Error code 7 (Void DP)
				}
				respDataSize = 1;
				break;
			}
			case 0xF6: // M_PropWrite.req
			{
				data[11] = 0xF5; // M_PropWrite.con
				if ((data[15] == 0x34) && (data[18] == 0x00))
				{
					respDataSize = -1;
				}
				else
				{
					data[16] = 0; // Number of elements must be 0 if negative response
					data[18] = 6; // Error code 6 (illegal command)
					respDataSize = 0;
					forceSend = true;
				}
				break;
			}
		}
	}

	if ((respDataSize != 0) || forceSend)
	{
		data[2] += respDataSize; // HID Report Header: Packet Length
		data[6] += respDataSize; // USB KNX Transfer Protocol Header: Body Length

		Serial1.print("TX HID report: len: ");
		Serial1.println((length + data[4] + 3) + respDataSize, DEC);

		for (int i = 0; i < ((length + data[4] + 3) + respDataSize); i++)
		{
			if (data[i] < 16)
				Serial1.print("0");
			Serial1.print(data[i], HEX);
			Serial1.print(" ");
		}
		Serial1.println("");
	}

	return respDataSize;
}

void sendKnxTunnelHidReport(uint8_t* data, uint16_t length)
{
	uint8_t buffer[length + 100];

	buffer[0]  = 0x01; // ReportID (fixed 0x01)
	buffer[1]  = 0x13; // PacketInfo must be 0x13 (SeqNo: 1, Type: 3)
	buffer[3]  = 0x00; // Protocol version (fixed 0x00)
	buffer[4]  = 0x08; // USB KNX Transfer Protocol Header Length (fixed 0x08)
	buffer[7]  = 0x01; // KNX Tunneling (0x01)
	buffer[8]  = 0x03; // EMI ID: 3 -> cEMI
	buffer[9]  = 0x00; // Manufacturer
	buffer[10] = 0x00; // Manufacturer
	
	// Copy cEMI frame to buffer
	memcpy(&buffer[11], data, length + 100);

	// We do not use reportId of the sendReport()-API here but instead provide it in the first byte of the buffer
    usb_hid.sendReport(0, data, length + 100);
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
static void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	// we don't use multiple report and report ID
	(void) report_id;
	(void) report_type;

	uint8_t tmpbuf[bufsize];
	memcpy(tmpbuf, buffer, bufsize);

	if (parseReport(tmpbuf, bufsize) > 0)
	{
		sendKnxTunnelHidReport(&tmpbuf[0], bufsize);
	}
}

// class UsbDataLinkLayer

UsbDataLinkLayer::UsbDataLinkLayer(DeviceObject& deviceObject, CemiServer& cemiServer, Platform& platform)
    : _deviceObject(deviceObject), _cemiServer(cemiServer), _platform(platform)
{
}

void UsbDataLinkLayer::loop()
{
    if (!_enabled)
	{
        return;
	}

	if (!isTxQueueEmpty())
	{
		uint8_t* buffer;
		uint16_t length;
		loadNextTxFrame(&buffer, &length);

		print("cEMI USB TX len: ");
		print(length);

		print(" data: ");
		printHex(" data: ", buffer, length);

		sendKnxTunnelHidReport(buffer, length);
	}

	if (!isRxQueueEmpty())
	{
		uint8_t* buffer;
		uint16_t length;
		loadNextRxFrame(&buffer, &length);

		// Prepare the cEMI frame
		CemiFrame frame(buffer, length);

		print("cEMI USB RX len: ");
		print(length);

		print(" data: ");
		printHex(" data: ", buffer, length);

		_cemiServer.frameReceived(frame);
	}
}

bool UsbDataLinkLayer::sendCemiFrame(CemiFrame& frame)
{
    if (!_enabled)
        return false;

    addFrameTxQueue(frame);

    return true;
}

void UsbDataLinkLayer::enabled(bool value)
{
    if (value && !_enabled)
    {
        usb_hid.enableOutEndpoint(true);
        usb_hid.setPollInterval(2);
        usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
        usb_hid.setReportCallback(NULL, set_report_callback);

        usb_hid.begin();

        // wait until device mounted
        while( !USBDevice.mounted() ) delay(1);

        _enabled = true;
        print("ownaddr ");
        println(_deviceObject.induvidualAddress(), HEX);
        return;
    }

    if (!value && _enabled)
    {
        println("USB data link layer cannot be disabled once enabled!");
        return;
    }
}

bool UsbDataLinkLayer::enabled() const
{
    return _enabled;
}

void UsbDataLinkLayer::addFrameTxQueue(CemiFrame& frame)
{
    _tx_queue_frame_t* tx_frame = new _tx_queue_frame_t;

    tx_frame->length = frame.totalLenght();
    tx_frame->data = new uint8_t[tx_frame->length];
    tx_frame->next = NULL;

/*
    print("cEMI USB TX len: ");
    print(tx_frame->length);

    printHex(" data:", tx_frame->data, tx_frame->length);
*/
    if (_tx_queue.back == NULL)
    {
        _tx_queue.front = _tx_queue.back = tx_frame;
    }
    else
    {
        _tx_queue.back->next = tx_frame;
        _tx_queue.back = tx_frame;
    }
}

bool UsbDataLinkLayer::isTxQueueEmpty()
{
    if (_tx_queue.front == NULL)
    {
        return true;
    }
    return false;
}

void UsbDataLinkLayer::loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength)
{
    if (_tx_queue.front == NULL)
    {
        return;
    }
    _tx_queue_frame_t* tx_frame = _tx_queue.front;
    *sendBuffer = tx_frame->data;
    *sendBufferLength = tx_frame->length;
    _tx_queue.front = tx_frame->next;

    if (_tx_queue.front == NULL)
    {
        _tx_queue.back = NULL;
    }
    delete tx_frame;
}
