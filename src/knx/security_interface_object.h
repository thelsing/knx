#pragma once

#include "config.h"
#ifdef USE_DATASECURE

#include "interface_object.h"

class SecureApplicationLayer;

class SecurityInterfaceObject: public InterfaceObject
{
public:
  SecurityInterfaceObject();

  void secureApplicationLayer(SecureApplicationLayer& secAppLayer);

  uint8_t* save(uint8_t* buffer) override;
  const uint8_t* restore(const uint8_t* buffer) override;
  uint16_t saveSize() override;

  bool isLoaded();

private:
  SecureApplicationLayer* _secAppLayer = nullptr;

  LoadState _state = LS_UNLOADED;

  // Our FDSK
  static uint8_t _fdsk[];
  static uint8_t _secReport[];
  static uint8_t _secReportCtrl[];
};
#endif
