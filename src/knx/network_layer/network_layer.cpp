#include "network_layer.h"

#include "../interface_object/device_object.h"
#include "../datalink_layer/data_link_layer.h"
#include "../transport_layer/tpdu.h"
#include "../bits.h"

NetworkLayer::NetworkLayer(DeviceObject& deviceObj, TransportLayer& layer) :
    _deviceObj(deviceObj),
    _transportLayer(layer)
{
    _hopCount = _deviceObj.defaultHopCount();
}

uint8_t NetworkLayer::hopCount() const
{
    return _hopCount;
}

bool NetworkLayer::isApciSystemBroadcast(APDU& apdu)
{
    switch (apdu.type())
    {
        // Application Layer Services on System Broadcast communication mode
        case SystemNetworkParameterRead:
        case SystemNetworkParameterResponse:
        case SystemNetworkParameterWrite:

        // Open media specific Application Layer Services on System Broadcast communication mode
        case DomainAddressSerialNumberRead:
        case DomainAddressSerialNumberResponse:
        case DomainAddressSerialNumberWrite:
        case DomainAddressRead:
        case DomainAddressSelectiveRead:
        case DomainAddressResponse:
        case DomainAddressWrite:
            return true;

        default:
            return false;
    }

    return false;
}

