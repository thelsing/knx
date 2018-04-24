#include "interface_object.h"

void InterfaceObject::readPropertyDescription(uint8_t& propertyId, uint8_t& propertyIndex, bool& writeEnable, uint8_t& type, uint16_t& numberOfElements, uint8_t& access)
{
    PropertyDescription* descriptions = propertyDescriptions();
    uint8_t count = propertyCount();
    
    PropertyDescription* desc = nullptr;
	
	// from KNX spec. 03.03.07 Application Layer (page 56) - 3.4.3.3  A_PropertyDescription_Read-service 
	// [...] and the Property of the object shall be addressed with 
	// a property_id OR with a property_index.The property_index shall be used ONLY if the property_id is zero. 
    // [...]	
	// If the property_id in the A_PropertyDescription_Read - PDU is NOT zero, then the field property_index 
	// shall be IGNORED; the remote application process shall use the indicated property_id to access the Property 
	// description.The property_index in the A_PropertyDescription_Response - PDU shall in this case be : 
	// - the correct value of the Property index of the addressed Property, or 
	// - the value of the field property_index of the received A_PropertyDescription_Read - PDU. 
	// For new implementations the property_index shall contain the correct value of the addressed Property. 
	// If the remote application process has a problem, e.g.Interface Object or Property does not exist, then the 
	// max_nr_of_elem of the A_PropertyDescription_Response - PDU shall be zero. 	
    if (propertyId != 0)
    {   
	    for (uint8_t i = 0; i < count; i++)
        {
            PropertyDescription d = descriptions[i];
            if (d.Id != propertyId)
                continue;
            
            desc = &d;
            propertyIndex = i;
            break;
        }
    }
    else
    {
	    // If the property_id in the A_PropertyDescription_Read - PDU is zero, the remote application process shall 
	    // use the indicated property_index to access the Property description.The property_index in the 
	    // A_PropertyDescription_Response - PDU shall be the value of the field property_index of the received 
	    // A_PropertyDescription_Read - PDU 	    
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
	else
	{
		numberOfElements = 0;
	}
}