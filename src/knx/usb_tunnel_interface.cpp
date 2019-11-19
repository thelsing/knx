#include "bits.h"
#include "usb_tunnel_interface.h"
#include "cemi_server.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define MAX_EP_SIZE 64
#define HID_HEADER_SIZE 3

//#define DEBUG_TX_HID_REPORT 
//#define DEBUG_RX_HID_REPORT 

extern bool sendKnxHidReport(uint8_t* data, uint16_t length);
extern bool isKnxHidSendReportPossible();

static const uint8_t descHidReport[] =
{
  //TUD_HID_REPORT_DESC_KNXHID_INOUT(64)
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

static uint16_t manufacturerId;
static uint16_t maskVersion;

struct _rx_queue_frame_t
{
    uint8_t* data;
    uint16_t length;
    _rx_queue_frame_t* next;
};

static struct _rx_queue_t
{
    _rx_queue_frame_t* front = nullptr;
    _rx_queue_frame_t* back = nullptr;
} _rx_queue;

const uint8_t* getKnxHidReportDescriptor()
{
	return &descHidReport[0];
}

uint16_t getHidReportDescriptorLength()
{
	return sizeof(descHidReport);
}

static void addFrameRxQueue(CemiFrame& frame)
{
    _rx_queue_frame_t* rx_frame = new _rx_queue_frame_t;

    rx_frame->length = frame.dataLength();
    rx_frame->data = new uint8_t[rx_frame->length];
    rx_frame->next = nullptr;

	memcpy(rx_frame->data, frame.data(), rx_frame->length);

/*
    print("cEMI USB RX len: ");
    print(rx_frame->length);

    printHex(" data:", rx_frame->data, rx_frame->length);
*/
    if (_rx_queue.back == nullptr)
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
    if (_rx_queue.front == nullptr)
    {
        return true;
    }
    return false;
}

static void loadNextRxFrame(uint8_t** receiveBuffer, uint16_t* receiveBufferLength)
{
    if (_rx_queue.front == nullptr)
    {
        return;
    }
    _rx_queue_frame_t* rx_frame = _rx_queue.front;
    *receiveBuffer = rx_frame->data;
    *receiveBufferLength = rx_frame->length;
    _rx_queue.front = rx_frame->next;

    if (_rx_queue.front == nullptr)
    {
        _rx_queue.back = nullptr;
    }
    delete rx_frame;
}

static void handleBusAccessServerProtocol(const uint8_t* requestData, uint16_t packetLength)
{
	if (packetLength > MAX_EP_SIZE)
		return;

	uint8_t data[MAX_EP_SIZE];
	memset(data, 0, sizeof(data));
	memcpy(data, requestData, packetLength);

	int8_t respDataSize = 0;

	uint8_t serviceId = data[8];
	switch (serviceId)
	{
		case 0x01: // Device Feature Get
		{
			data[8] = 0x02; // Device Feature Response

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
					pushWord(maskVersion, &data[12]); // USB KNX Transfer Protocol Body: Feature Data -> Mask version
					break;
				case 0x03: // Bus connection status
					Serial1.println("Device Feature Get: Bus connection status");
					respDataSize = 1;
					data[12] = 1; // USB KNX Transfer Protocol Body: Feature Data
					break;
				case 0x04: // KNX manufacturer code
					Serial1.println("Device Feature Get: KNX manufacturer code");
					respDataSize = 2;
					pushWord(manufacturerId, &data[12]); // USB KNX Transfer Protocol Body: Feature Data -> Manufacturer Code
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

	// Do we have to send a response?
	if (respDataSize > 0)
	{
		data[2] += respDataSize; // HID Report Header: Packet Length
		data[6] += respDataSize; // USB KNX Transfer Protocol Header: Body Length

#ifdef DEBUG_TX_HID_REPORT
		Serial1.print("TX HID report: len: ");
		Serial1.println((packetLength) + respDataSize, DEC);

		for (int i = 0; i < ((packetLength) + respDataSize); i++)
		{
			if (data[i] < 16)
				Serial1.print("0");
			Serial1.print(data[i], HEX);
			Serial1.print(" ");
		}
		Serial1.println("");
#endif
		sendKnxHidReport(data, MAX_EP_SIZE);
	}
}

static void sendKnxTunnelHidReport(uint8_t* data, uint16_t length)
{
	uint8_t buffer[length + 11];

	buffer[0]  = 0x01; // ReportID (fixed 0x01)
	buffer[1]  = 0x13; // PacketInfo must be 0x13 (SeqNo: 1, Type: 3)
	buffer[3]  = 0x00; // Protocol version (fixed 0x00)
	buffer[4]  = 0x08; // USB KNX Transfer Protocol Header Length (fixed 0x08)
	buffer[7]  = 0x01; // KNX Tunneling (0x01)
	buffer[8]  = 0x03; // EMI ID: 3 -> cEMI
	buffer[9]  = 0x00; // Manufacturer
	buffer[10] = 0x00; // Manufacturer
	
	// Copy cEMI frame to buffer
	memcpy(&buffer[11], data, length + HID_HEADER_SIZE + 8);

	buffer[2]  = 8 + length;      // KNX USB Transfer Protocol Header length (8, only first packet!) + cEMI length
	pushWord(length, &buffer[5]); // KNX USB Transfer Protocol Body length (cEMI length)

#ifdef DEBUG_TX_HID_REPORT
	Serial1.print("TX HID report: len: ");
	Serial1.println(buffer[2] + HID_HEADER_SIZE, DEC);

	for (int i = 0; i < (buffer[2] + HID_HEADER_SIZE); i++)
	{
		if (buffer[i] < 16)
			Serial1.print("0");
		Serial1.print(buffer[i], HEX);
		Serial1.print(" ");
	}
	Serial1.println("");
#endif

	sendKnxHidReport(buffer, MAX_EP_SIZE);
}

// Invoked when received SET_REPORT control request or via interrupt out pipe
void handleKnxHidReport(uint8_t const* data, uint16_t bufSize)
{
	if (bufSize!=MAX_EP_SIZE)
	return;

	if (data[0] == 0x01 && // ReportID (fixed 0x01)
		data[1] == 0x13)   // PacketInfo must be 0x13 (SeqNo: 1, Type: 3)
	{
		uint8_t packetLength = data[2];

#ifdef DEBUG_RX_HID_REPORT
		Serial1.print("RX HID report: len: ");
		Serial1.println(packetLength, DEC);

		for (int i = 0; i < (packetLength + HID_HEADER_SIZE); i++)
		{
			if (data[i] < 16)
				Serial1.print("0");
			Serial1.print(data[i], HEX);
			Serial1.print(" ");
		}
		Serial1.println("");
#endif

		if (data[3] == 0x00 && // Protocol version (fixed 0x00)
			data[4] == 0x08)   // USB KNX Transfer Protocol Header Length (fixed 0x08)
		{
			if (data[7] == 0x0F)   // Bus Access Server Feature (0x0F)
			{
				handleBusAccessServerProtocol(&data[0], packetLength + HID_HEADER_SIZE);
			}
			else if (data[7] == 0x01 && // KNX Tunneling (0x01)
					 data[8] == 0x03)   // EMI type: only cEMI supported
			{
				uint16_t bodyLength;
				popWord(bodyLength, (uint8_t*)&data[5]); // KNX USB Transfer Protocol Body length
				CemiFrame frame((uint8_t*)&data[11], bodyLength);
				addFrameRxQueue(frame);
			}
		}
	}
}

// class UsbTunnelInterface

UsbTunnelInterface::UsbTunnelInterface(CemiServer& cemiServer,
                                   uint16_t mId,
								   uint16_t mV)
    : _cemiServer(cemiServer)
{
	manufacturerId = mId;
	maskVersion = mV;
}

void UsbTunnelInterface::loop()
{
	// Make sure that the USB HW is also ready to send another report
	if (!isTxQueueEmpty() && isKnxHidSendReportPossible())
	{
		uint8_t* buffer;
		uint16_t length;
		loadNextTxFrame(&buffer, &length);
/*
		print("cEMI USB TX len: ");
		print(length);

		print(" data: ");
		printHex(" data: ", buffer, length);
*/
		sendKnxTunnelHidReport(buffer, length);
	}

	if (!isRxQueueEmpty())
	{
		uint8_t* buffer;
		uint16_t length;
		loadNextRxFrame(&buffer, &length);

		// Prepare the cEMI frame
		CemiFrame frame(buffer, length);
/*
		print("cEMI USB RX len: ");
		print(length);

		print(" data: ");
		printHex(" data: ", buffer, length);
*/
		_cemiServer.frameReceived(frame);
	}
}

bool UsbTunnelInterface::sendCemiFrame(CemiFrame& frame)
{
    addFrameTxQueue(frame);

    return true;
}

void UsbTunnelInterface::addFrameTxQueue(CemiFrame& frame)
{
    _tx_queue_frame_t* tx_frame = new _tx_queue_frame_t;

    tx_frame->length = frame.dataLength();
    tx_frame->data = new uint8_t[tx_frame->length];
    tx_frame->next = nullptr;

	memcpy(tx_frame->data, frame.data(), tx_frame->length);

/*
    print("cEMI USB TX len: ");
    print(tx_frame->length);

    printHex(" data:", tx_frame->data, tx_frame->length);
*/
    if (_tx_queue.back == nullptr)
    {
        _tx_queue.front = _tx_queue.back = tx_frame;
    }
    else
    {
        _tx_queue.back->next = tx_frame;
        _tx_queue.back = tx_frame;
    }
}

bool UsbTunnelInterface::isTxQueueEmpty()
{
    if (_tx_queue.front == nullptr)
    {
        return true;
    }
    return false;
}

void UsbTunnelInterface::loadNextTxFrame(uint8_t** sendBuffer, uint16_t* sendBufferLength)
{
    if (_tx_queue.front == nullptr)
    {
        return;
    }
    _tx_queue_frame_t* tx_frame = _tx_queue.front;
    *sendBuffer = tx_frame->data;
    *sendBufferLength = tx_frame->length;
    _tx_queue.front = tx_frame->next;

    if (_tx_queue.front == nullptr)
    {
        _tx_queue.back = nullptr;
    }
    delete tx_frame;
}
