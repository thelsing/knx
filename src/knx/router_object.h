#pragma once

#include "config.h"

#include "interface_object.h"
#include "knx_types.h"

class RouterObject: public InterfaceObject
{
public:
  RouterObject();

  virtual void masterReset(EraseCode eraseCode, uint8_t channel) override;

protected:
  void initializeProperties(size_t propertiesSize, Property** properties) override;

private:
  uint16_t getNumberOfElements(PropertyID propId);
};
