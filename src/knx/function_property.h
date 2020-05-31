#pragma once

#include "property.h"

class InterfaceObject;

template <class T> class FunctionProperty : public Property
{
  public:
    FunctionProperty(T* io, PropertyID id, uint8_t access,
                     uint8_t (*commandCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&),
                     uint8_t (*stateCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&))
        : Property(id, false, PDT_FUNCTION, 1, access), _interfaceObject(io), _commandCallback(commandCallback), _stateCallback(stateCallback)
        /* max_elements is set to 1, see 3.3.7 Application Layer p.68 */
    {}
    
    virtual uint8_t read(uint16_t start, uint8_t count, uint8_t* data) const override
    {
        return 0;
    }

    virtual uint8_t write(uint16_t start, uint8_t count, const uint8_t* data) override
    {
        return 0;
    }

    virtual uint8_t command(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) override
    {
        if (length == 0 || _commandCallback == nullptr )
            return 0xFF;

        return _commandCallback(_interfaceObject, data, length, resultData, resultLength);
    }

    virtual uint8_t state(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) override
    {
        if (length == 0 || _stateCallback == nullptr )
            return 0xFF;

        return _stateCallback(_interfaceObject, data, length, resultData, resultLength);
    }

  private:
    T* _interfaceObject = nullptr;
    uint8_t (*_commandCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&) = nullptr;
    uint8_t (*_stateCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&) = nullptr;
};
