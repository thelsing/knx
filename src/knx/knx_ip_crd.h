#pragma once

#include <cstdint>
#include "config.h"

#ifdef USE_IP

class KnxIpCRD
{
  public:
    KnxIpCRD(uint8_t* data);
    virtual ~KnxIpCRD();
    void address(uint16_t addr);
    uint16_t address() const;
    void type(uint8_t addr);
    uint8_t type() const;
    uint8_t length() const;
    void length(uint8_t value);

  protected:
    uint8_t* _data = 0;
};
#endif
