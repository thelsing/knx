#pragma once

#include "knx_types.h"
#include "stdint.h"
#include "npdu.h"
#include "tpdu.h"
#include "apdu.h"

#define NPDU_LPDU_DIFF 8
#define TPDU_NPDU_DIFF 1
#define APDU_TPDU_DIFF 0
#define TPDU_LPDU_DIFF (TPDU_NPDU_DIFF + NPDU_LPDU_DIFF)
#define APDU_LPDU_DIFF (APDU_TPDU_DIFF + TPDU_NPDU_DIFF + NPDU_LPDU_DIFF)

class CemiFrame
{
    friend class DataLinkLayer;
public:
    CemiFrame(uint8_t* data, uint16_t length);
    CemiFrame(uint8_t apduLength);
    CemiFrame(const CemiFrame& other);
    CemiFrame& operator= (CemiFrame other);

    MessageCode messageCode() const;
    void messageCode(MessageCode value);
    uint16_t totalLenght() const;
    
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
    AddressType addressType() const;
    void addressType(AddressType value);
    uint8_t hopCount() const;
    void hopCount(uint8_t value);
    uint16_t sourceAddress() const;
    void sourceAddress(uint16_t value);
    uint16_t destinationAddress() const;
    void destinationAddress(uint16_t value);

    NPDU& npdu();
    TPDU& tpdu();
    APDU& apdu();
    
    bool valid() const;
private:
    uint8_t buffer[0xff + NPDU_LPDU_DIFF]; //only valid of add info is zero
    uint8_t* _data;
    uint8_t* _ctrl1;
    NPDU _npdu;
    TPDU _tpdu;
    APDU _apdu;
    uint16_t _length; // only set if created from byte array
};