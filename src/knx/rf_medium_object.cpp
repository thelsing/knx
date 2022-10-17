#include "config.h"
#ifdef USE_RF

#include <cstring>
#include "rf_medium_object.h"
#include "bits.h"
#include "data_property.h"
#include "function_property.h"

RfMediumObject::RfMediumObject()
{
    uint8_t rfDomainAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // see KNX RF S-Mode AN160 p.11
    Property* properties[] =
    {
        new DataProperty(PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0, (uint16_t)OT_RF_MEDIUM),
        new DataProperty(PID_RF_MULTI_TYPE, true, PDT_GENERIC_01, 1, ReadLv3 | WriteLv2, (uint8_t)0x00),
        new DataProperty(PID_RF_RETRANSMITTER, false, PDT_GENERIC_01, 1, ReadLv3 | WriteLv0, (uint8_t)0x00),
        new DataProperty(PID_RF_DOMAIN_ADDRESS, true, PDT_GENERIC_06, 1, ReadLv3 | WriteLv2, rfDomainAddress),
        new FunctionProperty<RfMediumObject>(this, PID_RF_BIDIR_TIMEOUT, 
            [](RfMediumObject* io, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
            {
                resultData[0] = 0x00; // success
                resultData[1] = 0xFF; // permanent bidirectional device
                resultData[2] = 0xFF; // permanent bidirectional device
                resultLength = 3;
            }, 
            [](RfMediumObject* io, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
            {
                resultData[0] = 0x00; // success
                resultData[1] = 0xFF; // permanent bidirectional device
                resultData[2] = 0xFF; // permanent bidirectional device
                resultLength = 3;
            }), 
/*      These properties are used in NMP_LinkBudget_Measure to diagnose the Link Budget of the communication.
        This in not implemented yet.
        new DataProperty(PID_RF_DIAG_SA_FILTER_TABLE, true, PDT_GENERIC_03, 8, ReadLv3 | WriteLv3),
        new DataProperty(PID_RF_DIAG_BUDGET_TABLE, false, PDT_GENERIC_03, 8, ReadLv3 | WriteLv0),
        new FunctionProperty<RfMediumObject>(this, PID_RF_DIAG_PROBE, 
            [](RfMediumObject* io, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
            {
            }, 
            [](RfMediumObject* io, uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength)
            {
            }), */
    };
    initializeProperties(sizeof(properties), properties);
}

const uint8_t* RfMediumObject::rfDomainAddress()
{
    DataProperty* prop = (DataProperty*)property(PID_RF_DOMAIN_ADDRESS);
    return prop->data();
}

void RfMediumObject::rfDomainAddress(const uint8_t* value)
{
    Property* prop = property(PID_RF_DOMAIN_ADDRESS);
    prop->write(value);
}
#endif
