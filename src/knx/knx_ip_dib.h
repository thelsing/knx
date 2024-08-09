#pragma once

#include <cstdint>
#include "config.h"

#ifdef USE_IP

enum DescriptionTypeCode : uint8_t
{
    DEVICE_INFO = 0x01,
    SUPP_SVC_FAMILIES = 0x02,
    IP_CONFIG = 0x03,
    IP_CUR_CONFIG = 0x04,
    KNX_ADDRESSES = 0x05,
    MANUFACTURER_DATA = 0x06,
    TUNNELING_INFO = 0x07,
    EXTENDED_DEVICE_INFO = 0x08,
    MFR_DATA = 0xFE
};

class KnxIpDIB
{
  public:
    KnxIpDIB(uint8_t* data);
    virtual ~KnxIpDIB();
    DescriptionTypeCode code() const;
    void code(DescriptionTypeCode value);
    uint8_t length() const;
    void length(uint8_t value);

  protected:
    uint8_t* _data = 0;
};
#endif
