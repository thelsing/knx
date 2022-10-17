#include "network_layer.h"
#include "device_object.h"
#include "data_link_layer.h"
#include "tpdu.h"
#include "cemi_frame.h"
#include "bits.h"
#include "apdu.h"

NetworkLayer::NetworkLayer(DeviceObject &deviceObj, TransportLayer& layer) :
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

