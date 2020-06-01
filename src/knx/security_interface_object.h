#pragma once

#include "config.h"
#ifdef USE_DATASECURE

#include "interface_object.h"

class SecurityInterfaceObject: public InterfaceObject
{
public:
  SecurityInterfaceObject();
};
#endif
