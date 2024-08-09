#pragma once

#include <cstdint>
#include "config.h"

#ifdef USE_IP

#define LEN_CRI 4

//TODO vervollständigen
enum ConnectionType : uint8_t
{
  DEVICE_MGMT_CONNECTION = 3,
  TUNNEL_CONNECTION = 4,
  REMLOG_CONNECTION = 6,
  REMCONF_CONNECTION = 7,
  OBJSVR_CONNECTION = 8
};

// Connection Request Information
class KnxIpCRI
{
  public:
    KnxIpCRI(uint8_t* data);
    virtual ~KnxIpCRI();
    ConnectionType type() const;
    void type(ConnectionType value);
    void layer(uint8_t layer);
    uint8_t layer() const;
    uint8_t length() const;
    void length(uint8_t value);

  protected:
    uint8_t* _data = 0;
};
#endif
