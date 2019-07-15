#pragma once

#include "interface_object.h"
#include "device_object.h"
#include "platform.h"

class IpParameterObject : public InterfaceObject
{
  public:
    IpParameterObject(DeviceObject& deviceObject, Platform& platform);
    void readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data);
    void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    uint8_t propertySize(PropertyID id);

    uint8_t* save(uint8_t* buffer);
    uint8_t* restore(uint8_t* buffer);

    uint32_t multicastAddress() const;
    uint8_t ttl() const { return _ttl; }

  protected:
    uint8_t propertyCount();
    PropertyDescription* propertyDescriptions();

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

    void loadEvent(uint8_t* data);
    void loadEventUnloaded(uint8_t* data);
    void loadEventLoading(uint8_t* data);
    void loadEventLoaded(uint8_t* data);
    void loadEventError(uint8_t* data);
    void additionalLoadControls(uint8_t* data);
    void loadState(LoadState newState);
    LoadState _state = LS_UNLOADED;
    ErrorCode _errorCode = E_NO_FAULT;
};