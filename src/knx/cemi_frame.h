#pragma once

#include "knx_types.h"
#include "stdint.h"
#include "npdu.h"
#include "tpdu.h"
#include "apdu.h"
#include "config.h"

#define NPDU_LPDU_DIFF 8
#define TPDU_NPDU_DIFF 1
#define APDU_TPDU_DIFF 0
#define TPDU_LPDU_DIFF (TPDU_NPDU_DIFF + NPDU_LPDU_DIFF)
#define APDU_LPDU_DIFF (APDU_TPDU_DIFF + TPDU_NPDU_DIFF + NPDU_LPDU_DIFF)

// Mesg Code and additional info length
#define CEMI_HEADER_SIZE 2

class CemiFrame
{
    friend class DataLinkLayer;

  public:
    CemiFrame(uint8_t* data, uint16_t length);
    CemiFrame(uint8_t apduLength);
    CemiFrame(const CemiFrame& other);
    CemiFrame& operator=(CemiFrame other);

    MessageCode messageCode() const;
    void messageCode(MessageCode value);
    uint16_t totalLenght() const;
    uint16_t telegramLengthtTP() const;
    void fillTelegramTP(uint8_t* data);
    uint16_t telegramLengthtRF() const;
    void fillTelegramRF(uint8_t* data);
    uint8_t* data();
    uint16_t dataLength();

    FrameFormat frameType() const;
    void frameType(FrameFormat value);
    Repetition repetition() const;
    void repetition(Repetition value);
    SystemBroadcast systemBroadcast() const;
    void systemBroadcast(SystemBroadcast value);
    Priority priority() const;
    void priority(Priority value);
    AckType ack() const;
    void ack(AckType value);
    Confirm confirm() const;
    void confirm(Confirm value);
    AddressType addressType() const;
    void addressType(AddressType value);
    uint8_t hopCount() const;
    void hopCount(uint8_t value);
    uint16_t sourceAddress() const;
    void sourceAddress(uint16_t value);
    uint16_t destinationAddress() const;
    void destinationAddress(uint16_t value);

#ifdef USE_RF
    // only for RF medium
    uint8_t* rfSerialOrDoA() const;
    void rfSerialOrDoA(const uint8_t* rfSerialOrDoA);
    uint8_t rfInfo() const;
    void rfInfo(uint8_t rfInfo);
    uint8_t rfLfn() const;
    void rfLfn(uint8_t rfInfo);
#endif
    NPDU& npdu();
    TPDU& tpdu();
    APDU& apdu();

    uint8_t calcCrcTP(uint8_t* buffer, uint16_t len);
    bool valid() const;

  private:
    uint8_t buffer[0xff + NPDU_LPDU_DIFF] = {0}; //only valid of add info is zero
    uint8_t* _data = 0;
    uint8_t* _ctrl1 = 0;
    NPDU _npdu;
    TPDU _tpdu;
    APDU _apdu;
    uint16_t _length = 0; // only set if created from byte array

#ifdef USE_RF
    // FIXME: integrate this propery in _data
    // only for RF medium
    uint8_t* _rfSerialOrDoA = 0;
    uint8_t  _rfInfo = 0;
    uint8_t  _rfLfn = 0xFF; // RF Data Link layer frame number
 #endif

    uint8_t _sourceInterfaceIndex;
};
