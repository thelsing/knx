#include "config.h"
#ifdef USE_USB

#include "bits.h"
#include "usb_tunnel_interface.h"
#include "cemi_server.h"
#include "cemi_frame.h"

#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a < b) ? (a) : (b))

#define MAX_EP_SIZE 64
#define HID_HEADER_SIZE 3
#define MAX_KNX_TELEGRAM_SIZE 263
#define KNX_HID_REPORT_ID 0x01
#define PROTOCOL_VERSION 0x00
#define PROTOCOL_HEADER_LENGTH 0x08

// Maximum possible payload data bytes in a transfer protocol body
#define MAX_DATASIZE_START_PACKET 52
#define MAX_DATASIZE_PARTIAL_PACKET 61

#define PACKET_TYPE_START 1
#define PACKET_TYPE_END 2
#define PACKET_TYPE_PARTIAL 4

//#define DEBUG_TX_HID_REPORT 
//#define DEBUG_RX_HID_REPORT 

extern bool sendHidReport(uint8_t* data, uint16_t length);
extern bool isSendHidReportPossible();

// class UsbTunnelInterface

UsbTunnelInterface::UsbTunnelInterface(CemiServer& cemiServer,
                                   uint16_t mId,
								   uint16_t mV)
    : _cemiServer(cemiServer),
	  _manufacturerId(mId),
	  _maskVersion(mV)
{
}

void UsbTunnelInterface::loop()
{
	// Make sure that the USB HW is also ready to send another report
	if (!isTxQueueEmpty() && isSendHidReportPossible())
	{
		uint8_t* buffer;
		uint16_t length;
		loadNextTxFrame(&buffer, &length);
		sendHidReport(buffer, length);
		delete buffer;
	}

	// Check if we already a COMPLETE transport protocol packet
	// A transport protocol packet might be split into multiple HID reports and
	// need to be assembled again
	if (rxHaveCompletePacket)
	{
		handleHidReportRxQueue();
		rxHaveCompletePacket = false;
	}
}

/* USB TX */

void UsbTunnelInterface::sendCemiFrame(CemiFrame& frame)
{
	sendKnxHidReport(KnxTunneling, ServiceIdNotUsed, frame.data(), frame.dataLength());
}

void UsbTunnelInterface::addBufferTxQueue(uint8_t* data, uint16_t length)
{
    _queue_buffer_t* tx_buffer = new _queue_buffer_t;

    tx_buffer->length = MAX_EP_SIZE;
    tx_buffer->data = new uint8_t[MAX_EP_SIZE]; // We always have to send full max. USB endpoint size of 64 bytes
    tx_buffer->next = nullptr;

	memcpy(tx_buffer->data, data, tx_buffer->length);
	memset(&tx_buffer->data[length], 0x00, MAX_EP_SIZE - length); // Set unused bytes to zero

    if (_tx_queue.back == nullptr)
    {
        _tx_queue.front = _tx_queue.back = tx_buffer;
    }
    else
    {
        _tx_queue.back->next = tx_buffer;
        _tx_queue.back = tx_buffer;
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
    _queue_buffer_t* tx_buffer = _tx_queue.front;
    *sendBuffer = tx_buffer->data;
    *sendBufferLength = tx_buffer->length;
    _tx_queue.front = tx_buffer->next;

    if (_tx_queue.front == nullptr)
    {
        _tx_queue.back = nullptr;
    }
    delete tx_buffer;

#ifdef DEBUG_TX_HID_REPORT
	print("TX HID report: len: ");
	// We do not print the padded zeros
	uint8_t len = (*sendBuffer)[2];
	println(len, DEC);

	for (int i = 0; i < len; i++)
	{
		if ((*sendBuffer)[i] < 16)
			print("0");
		print((*sendBuffer)[i], HEX);
		print(" ");
	}
	println("");
#endif
}

void UsbTunnelInterface::sendKnxHidReport(ProtocolIdType protId, ServiceIdType servId, uint8_t* data, uint16_t length)
{
	uint16_t maxData = MAX_DATASIZE_START_PACKET;
	uint8_t packetType = PACKET_TYPE_START;

	if (length > maxData)
	{
		packetType |= PACKET_TYPE_PARTIAL;
	}

	uint16_t offset = 0;
	uint8_t* buffer = nullptr;

	// In theory we can only have sequence numbers from 1..5
	// First packet: 51 bytes max
	// Other packets: 62 bytes max.
	// -> 51 + 4*62 = 296 bytes -> enough for a KNX cEMI extended frame APDU + Transport Protocol Header length
	for(uint8_t seqNum = 1; seqNum < 6; seqNum++)
	{
		uint16_t copyLen = MIN(length, maxData);

		// If this is the first packet we include the transport protocol header
		if (packetType & PACKET_TYPE_START)
		{
			buffer = new uint8_t[copyLen + 8 + HID_HEADER_SIZE]; // length of transport protocol header: 11 bytes
			buffer[2]  = 8 + copyLen; // KNX USB Transfer Protocol Body length		
			buffer[3]  = PROTOCOL_VERSION; // Protocol version (fixed 0x00)
			buffer[4]  = PROTOCOL_HEADER_LENGTH; // USB KNX Transfer Protocol Header Length (fixed 0x08)
			pushWord(copyLen, &buffer[5]); // KNX USB Transfer Protocol Body length (e.g. cEMI length)			
			buffer[7]  = (uint8_t) protId; // KNX Tunneling (0x01) or KNX Bus Access Server (0x0f)
			buffer[8]  = (protId == KnxTunneling) ? (uint8_t)CEMI : (uint8_t)servId; // either EMI ID or Service Id
			buffer[9]  = 0x00; // Manufacturer (fixed 0x00) see KNX Spec 9/3 p.23 3.4.1.3.5
			buffer[10] = 0x00; // Manufacturer (fixed 0x00) see KNX Spec 9/3 p.23 3.4.1.3.5
			memcpy(&buffer[11], &data[offset], copyLen); // Copy payload for KNX USB Transfer Protocol Body
		}
		else
		{
			buffer = new uint8_t[copyLen]; // no transport protocol header in partial packets
			buffer[2] = copyLen; // KNX USB Transfer Protocol Body length
			memcpy(&buffer[0], &data[offset], copyLen); // Copy payload for KNX USB Transfer Protocol Body
		}

		offset += copyLen;
		if (offset >= length)
		{
			packetType |= PACKET_TYPE_END;
		}

		buffer[0]  = KNX_HID_REPORT_ID; // ReportID (fixed 0x01)
		buffer[1]  = ((seqNum << 4) & 0xF0) | (packetType & 0x07); // PacketInfo (SeqNo and Type)

		addBufferTxQueue(buffer, (buffer[2] + HID_HEADER_SIZE));

		delete[] buffer;

		if (offset >= length)
		{
			break;
		}

		packetType &= ~PACKET_TYPE_START;
		maxData = MAX_DATASIZE_PARTIAL_PACKET;
	}
}

/* USB RX */

// Invoked when received SET_REPORT control request or via interrupt out pipe
void UsbTunnelInterface::receiveHidReport(uint8_t const* data, uint16_t bufSize)
{
	// Check KNX ReportID (fixed 0x01)
	if (data[0] == KNX_HID_REPORT_ID)
	{
		// We just store only the used space of the HID report buffer
		// which is normally padded with 0 to fill the complete USB EP size (e.g. 64 bytes)
		uint8_t packetLength = data[2] + HID_HEADER_SIZE;
		UsbTunnelInterface::addBufferRxQueue(data, packetLength);

		// Check if packet type indicates last packet
		if ((data[1] & PACKET_TYPE_END) == PACKET_TYPE_END)
		{
			// Signal main loop that we have a complete KNX USB packet
			rxHaveCompletePacket = true;
		}
	}
}

UsbTunnelInterface::_queue_t UsbTunnelInterface::_rx_queue;
bool UsbTunnelInterface::rxHaveCompletePacket = false;

void UsbTunnelInterface::addBufferRxQueue(const uint8_t* data, uint16_t length)
{
    _queue_buffer_t* rx_buffer = new _queue_buffer_t;

    rx_buffer->length = length;
    rx_buffer->data = new uint8_t[rx_buffer->length];
    rx_buffer->next = nullptr;

	memcpy(rx_buffer->data, data, rx_buffer->length);

    if (_rx_queue.back == nullptr)
    {
        _rx_queue.front =_rx_queue.back = rx_buffer;
    }
    else
    {
        _rx_queue.back->next = rx_buffer;
        _rx_queue.back = rx_buffer;
    }
}

bool UsbTunnelInterface::isRxQueueEmpty()
{
    if (_rx_queue.front == nullptr)
    {
        return true;
    }
    return false;
}

void UsbTunnelInterface::loadNextRxBuffer(uint8_t** receiveBuffer, uint16_t* receiveBufferLength)
{
    if (_rx_queue.front == nullptr)
    {
        return;
    }
    _queue_buffer_t* rx_buffer = _rx_queue.front;
    *receiveBuffer = rx_buffer->data;
    *receiveBufferLength = rx_buffer->length;
    _rx_queue.front = rx_buffer->next;

    if (_rx_queue.front == nullptr)
    {
        _rx_queue.back = nullptr;
    }
    delete rx_buffer;

#ifdef DEBUG_RX_HID_REPORT
	print("RX HID report: len: ");
	println(*receiveBufferLength, DEC);

	for (int i = 0; i < (*receiveBufferLength); i++)
	{
		if ((*receiveBuffer)[i] < 16)
			print("0");
		print((*receiveBuffer)[i], HEX);
		print(" ");
	}
	println("");
#endif
}

void UsbTunnelInterface::handleTransferProtocolPacket(uint8_t* data, uint16_t length)
{
	if (data[0] == PROTOCOL_VERSION && // Protocol version (fixed 0x00)
		data[1] == PROTOCOL_HEADER_LENGTH)   // USB KNX Transfer Protocol Header Length (fixed 0x08)
	{
		uint16_t bodyLength;
		popWord(bodyLength, (uint8_t*)&data[2]); // KNX USB Transfer Protocol Body length

		if (data[4] == (uint8_t) BusAccessServer)   // Bus Access Server Feature (0x0F)
		{
			handleBusAccessServerProtocol((ServiceIdType)data[5], &data[8], bodyLength);
		}
		else if (data[4] == (uint8_t) KnxTunneling) // KNX Tunneling (0x01)
		{
			if (data[5] == (uint8_t) CEMI)   // EMI type: only cEMI supported (0x03))
			{
				// Prepare the cEMI frame
				CemiFrame frame((uint8_t*)&data[8], bodyLength);
		/*
				print("cEMI USB RX len: ");
				print(length);

				print(" data: ");
				printHex(" data: ", buffer, length);
		*/
				_cemiServer.frameReceived(frame);
			}
			else
			{
				println("Error: Only cEMI is supported!");
			}
		}
	}
}

void UsbTunnelInterface::handleHidReportRxQueue()
{
	if (isRxQueueEmpty())
	{
		println("Error: RX HID report queue was empty!");
		return;
	}

	uint8_t tpPacket[MAX_KNX_TELEGRAM_SIZE + PROTOCOL_HEADER_LENGTH]; // Transport Protocol Header + Body
	uint16_t offset = 0;
	bool success = false;

	// Now we have to reassemble the whole transport protocol packet which might be distributed over multiple HID reports

	// In theory we can only have sequence numbers from 1..5
	// First packet: 51 bytes max
	// Other packets: 62 bytes max.
	// -> 51 + 4*62 = 296 bytes -> enough for a KNX cEMI extended frame APDU + Transport Protocol Header length
	for(int expSeqNum = 1; expSeqNum < 6; expSeqNum++)
	{
		// We should have at least one packet: either single packet (START and END set) or
		// start packet (START and PARTIAL set) -> thus load first part
		uint8_t* data;
		uint16_t bufSize;
		loadNextRxBuffer(&data, &bufSize); // bufSize contains the complete HID report length incl. HID header

		// Get KNX HID report header details
		uint8_t seqNum = data[1] >> 4;
		uint8_t packetType = data[1] & 0x07;
		uint8_t packetLength = MIN(data[2], bufSize - HID_HEADER_SIZE); // Do not try to read more than we actually have!

		// Does the received sequence number match the expected one?
		if (expSeqNum != seqNum)
		{
			println("Error: Wrong sequence number!");
			delete data;
			continue;
		}

		// first RX buffer from queue should contain the first part of the transfer protocol packet
		if ((expSeqNum == 1) && ((packetType & PACKET_TYPE_START) != PACKET_TYPE_START))
		{
			println("Error: Sequence number 1 does not contain a START packet!");
			delete data;
			continue;
		}

		// Make sure we only have one START packet
		if ((expSeqNum != 1) && ((packetType & PACKET_TYPE_START) == PACKET_TYPE_START))
		{
			println("Error: Sequence number (!=1) contains a START packet!");
			delete data;
			continue;
		}

		// Make sure other packets are marked correctly as PARTIAL packet
		if ((expSeqNum != 1) && ((packetType & PACKET_TYPE_PARTIAL) != PACKET_TYPE_PARTIAL))
		{
			println("Error: Sequence number (!=1) must be a PARTIAL packet!");
			delete data;
			continue;
		}

		// Not really necessary, but we reset the offset here to zero
		if ((packetType & PACKET_TYPE_START) == PACKET_TYPE_START)
		{
			offset = 0;
		}

		// Copy KNX HID Report Body to final buffer for concatenating
		memcpy(&tpPacket[offset], &data[3], packetLength);
		// Remove the source HID report buffer
		delete data;
		// Move offset
		offset += packetLength;

		// If we reached the end of the transport protocol packet, leave the loop
		if ((packetType & PACKET_TYPE_END) == PACKET_TYPE_END)
		{
			success = true;
			break;
		}
	}

	// Make sure that we really saw the end of the transport protocol packet
	if (success)
	{
		handleTransferProtocolPacket(tpPacket, offset);
	}
	else
	{
		println("Error: Did not find END packet!");
	}
}

void UsbTunnelInterface::handleBusAccessServerProtocol(ServiceIdType servId, const uint8_t* requestData, uint16_t packetLength)
{
	uint8_t respData[3]; // max. 3 bytes are required for a response

	switch (servId)
	{
		case DeviceFeatureGet: // Device Feature Get
		{
			FeatureIdType featureId = (FeatureIdType)requestData[0];
			respData[0] = (uint8_t) featureId; // first byte in repsonse is the featureId itself again

			switch (featureId)
			{
				case SupportedEmiType: // Supported EMI types
					println("Device Feature Get: Supported EMI types");
					respData[1] = 0x00; // USB KNX Transfer Protocol Body: Feature Data
					respData[2] = 0x04; // USB KNX Transfer Protocol Body: Feature Data -> only cEMI supported
					sendKnxHidReport(BusAccessServer, DeviceFeatureResponse, respData, 3);
					break;
				case HostDeviceDescriptorType0: // Host Device Descriptor Type 0
					println("Device Feature Get: Host Device Descriptor Type 0");
					pushWord(_maskVersion, &respData[1]); // USB KNX Transfer Protocol Body: Feature Data -> Mask version
					sendKnxHidReport(BusAccessServer, DeviceFeatureResponse, respData, 3);
					break;
				case BusConnectionStatus: // Bus connection status
					println("Device Feature Get: Bus connection status");
					respData[1] = 1; // USB KNX Transfer Protocol Body: Feature Data -> bus connection status
					sendKnxHidReport(BusAccessServer, DeviceFeatureResponse, respData, 2);
					break;
				case KnxManufacturerCode: // KNX manufacturer code
					println("Device Feature Get: KNX manufacturer code");
					pushWord(_manufacturerId, &respData[1]); // USB KNX Transfer Protocol Body: Feature Data -> Manufacturer Code
					sendKnxHidReport(BusAccessServer, DeviceFeatureResponse, respData, 3);
					break;
				case ActiveEmiType: // Active EMI type
					println("Device Feature Get: Active EMI type");
					respData[1] = (uint8_t) CEMI; // USB KNX Transfer Protocol Body: Feature Data -> cEMI type ID
					sendKnxHidReport(BusAccessServer, DeviceFeatureResponse, respData, 2);
					break;
				default:
					break;
			}
			break;
		}
		case DeviceFeatureSet: // Device Feature Set
		{
			FeatureIdType featureId = (FeatureIdType)requestData[0];
			switch (featureId)
			{
				case ActiveEmiType: // Active EMI type
					print("Device Feature Set: Active EMI type: ");
					if (requestData[1] < 16)
						print("0");
					println(requestData[1], HEX); // USB KNX Transfer Protocol Body: Feature Data -> EMI TYPE ID
					break;
				// All other featureIds must not be set
				case SupportedEmiType: // Supported EMI types
				case HostDeviceDescriptorType0: // Host Device Descriptor Type 0
				case BusConnectionStatus: // Bus connection status
				case KnxManufacturerCode: // KNX manufacturer code
				default:
					break;
			}
			break;
		}

		// These are only sent from the device to the host
		case DeviceFeatureResponse: // Device Feature Response
		case DeviceFeatureInfo:     // Device Feature Info
		case DeviceFeatureEscape:   // reserved (ESCAPE for future extensions)
		default:
			break;
	}
}

/* USB HID report descriptor for KNX HID */

const uint8_t UsbTunnelInterface::descHidReport[] =
{
  //TUD_HID_REPORT_DESC_KNXHID_INOUT(64)
0x06, 0xA0, 0xFF,  // Usage Page (Vendor Defined 0xFFA0)
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

const uint8_t* UsbTunnelInterface::getKnxHidReportDescriptor()
{
	return &descHidReport[0];
}

uint16_t UsbTunnelInterface::getHidReportDescriptorLength()
{
	return sizeof(descHidReport);
}

#endif
