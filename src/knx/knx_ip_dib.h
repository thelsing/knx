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
    MFR_DATA = 0xFE
};

class DIB
{
  public:
    DIB(uint8_t* data);
    DescriptionTypeCode code();
    uint8_t length();
  private:
    uint8_t* _data = 0;
};
#endif
