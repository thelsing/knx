#pragma once

#include <cstdint>
#include "config.h"

#ifdef USE_IP

#define LEN_CH 4

// Connection Header
class KnxIpCH
{
  public:
    KnxIpCH(uint8_t* data);
    virtual ~KnxIpCH();
    void channelId(uint8_t channelId);
    uint8_t channelId() const;
    void sequenceCounter(uint8_t sequenceCounter);
    uint8_t sequenceCounter() const;
    void status(uint8_t status);
    uint8_t status() const;
    void length(uint8_t value);
    uint8_t length() const;

  protected:
    uint8_t* _data = 0;
};
#endif
