#pragma once

#include "interface_object.h"
#include "device_object.h"
#include "platform.h"

class IpParameterObject : public InterfaceObject
{
  public:
    IpParameterObject(DeviceObject& deviceObject, Platform& platform);
    void readProperty(PropertyID id, uint16_t start, uint8_t& count, uint8_t* data) override;
    void writeProperty(PropertyID id, uint16_t start, uint8_t* data, uint8_t& count) override;
    uint8_t propertySize(PropertyID id) override;
    ObjectType objectType() override
    {
        return OT_IP_PARAMETER;
    }

    uint8_t* save(uint8_t* buffer) override;
    uint8_t* restore(uint8_t* buffer) override;
    uint16_t saveSize() override;

    uint32_t multicastAddress() const;
    uint8_t ttl() const { return _ttl; }

  protected:
    uint8_t propertyDescriptionCount() override;
    PropertyDescription* propertyDescriptions() override;

  private:
    uint16_t _projectInstallationId = 0;
    uint8_t _ipAssignmentMethod = 0;
    uint8_t _ipCapabilities = 0;
    uint32_t _ipAddress = 0;
    uint32_t _subnetMask = 0;
    uint32_t _defaultGateway = 0;
    uint32_t _multicastAddress = 0;
    uint8_t _ttl = 60;
    char _friendlyName[30] = {0};
    DeviceObject& _deviceObject;
    Platform& _platform;
};