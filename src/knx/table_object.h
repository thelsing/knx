#pragma once

#include "interface_object.h"
#include "platform.h"

class TableObject: public InterfaceObject
{
public:
    TableObject(Platform& platform);
    virtual void readProperty(PropertyID id, uint32_t start, uint32_t count, uint8_t* data);
    virtual void writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count);
    virtual uint8_t propertySize(PropertyID id);
    virtual ~TableObject();
    LoadState loadState();
    virtual uint8_t* save(uint8_t* buffer);
    virtual uint8_t* restore(uint8_t* buffer);
protected:
    virtual void beforeStateChange(LoadState& newState) {}
    uint8_t* _data = 0;
    uint32_t _size = 0;
    ErrorCode _errorCode = E_NO_FAULT;
private:
    uint32_t tableReference();
    bool allocTable(uint32_t size, bool doFill, uint8_t fillByte);
    void loadEvent(uint8_t* data);
    void loadEventUnloaded(uint8_t* data);
    void loadEventLoading(uint8_t* data);
    void loadEventLoaded(uint8_t* data);
    void loadEventError(uint8_t* data);
    void additionalLoadControls(uint8_t* data);
    void loadState(LoadState newState);
    LoadState _state = LS_UNLOADED;
    Platform& _platform;
};
