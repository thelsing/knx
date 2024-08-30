#pragma once

#include "../interface_object/interface_object.h"

namespace Knx
{
    class CemiServerObject: public InterfaceObject
    {
        public:
            CemiServerObject();

            void setMediumTypeAsSupported(DptMedium dptMedium);
            void clearSupportedMediaTypes();
            const char* name() override
            {
                return "CemiServerObject";
            }
    };
}