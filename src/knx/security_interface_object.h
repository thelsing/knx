#pragma once

#include "config.h"
#ifdef USE_DATASECURE

#include "interface_object.h"

class SecurityInterfaceObject: public InterfaceObject
{
public:
  SecurityInterfaceObject();
  uint8_t* save(uint8_t* buffer) override;
  const uint8_t* restore(const uint8_t* buffer) override;
  uint16_t saveSize() override;

  bool isLoaded();

private:
  LoadState _state = LS_UNLOADED;

  // Our FDSK
  static uint8_t _fdsk[];
};
#endif
