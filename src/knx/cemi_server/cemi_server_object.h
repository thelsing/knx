#pragma once

#include "../interface_object/interface_object.h"

class CemiServerObject: public InterfaceObject
{
    public:
        CemiServerObject();

        void setMediumTypeAsSupported(DptMedium dptMedium);
        void clearSupportedMediaTypes();
};
