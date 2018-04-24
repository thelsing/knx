#include "interface_object.h"

void InterfaceObject::readPropertyDescription(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access)
{
    PropertyDescription* descriptions = propertyDescriptions();
    uint8_t count = propertyCount();
    
    PropertyDescription* desc = nullptr;
    if (propertyId != 0)
    {
        
        for (uint8_t i = 0; i < count; i++)
        {
            PropertyDescription d = descriptions[propertyIndex];
            if (d.Id != propertyId)
                continue;
            
            desc = &d;
            propertyIndex = i;
            break;
        }
    }
    else
    {
        if (propertyIndex >= 0 && propertyIndex < count) 
        {
            desc = &descriptions[propertyIndex];
        }
    }
    
    if (desc != nullptr)
    {
	    propertyId = desc->Id;
        writeEnable = desc->WriteEnable;
        type = desc->Type;
        numberOfElements = desc->MaxElements;
        access = desc->Access;        
    }
}