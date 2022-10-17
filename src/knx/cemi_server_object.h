#pragma once

#include "config.h"
#ifdef USE_CEMI_SERVER

#include "interface_object.h"

class CemiServerObject: public InterfaceObject
{
public:
  CemiServerObject();

  void setMediumTypeAsSupported(DptMedium dptMedium);
  void clearSupportedMediaTypes();
};

#endif
