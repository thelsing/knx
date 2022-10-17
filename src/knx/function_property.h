#pragma once

#include "property.h"

class InterfaceObject;

template <class T> class FunctionProperty : public Property
{
  public:
    FunctionProperty(T* io, PropertyID id, 
                     void (*commandCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&),
                     void (*stateCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&))
        : Property(id, false, PDT_FUNCTION, 1, ReadLv0|WriteLv0), _interfaceObject(io), _commandCallback(commandCallback), _stateCallback(stateCallback)
        /* max_elements is set to 1, read and write level any value so we use Lv0, see 3.3.7 Application Layer p.68 */
    {}
    
    uint8_t read(uint16_t start, uint8_t count, uint8_t* data) const override
    {
        return 0;
    }

    uint8_t write(uint16_t start, uint8_t count, const uint8_t* data) override
    {
        return 0;
    }

    void command(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) override
    {
        if (length == 0 || _commandCallback == nullptr )
        {
            resultLength = 0;
            return;
        }
        _commandCallback(_interfaceObject, data, length, resultData, resultLength);
    }

    void state(uint8_t* data, uint8_t length, uint8_t* resultData, uint8_t& resultLength) override
    {
        if (length == 0 || _stateCallback == nullptr )
        {
            resultLength = 0;
            return;
        }
        _stateCallback(_interfaceObject, data, length, resultData, resultLength);
    }

  private:
    T* _interfaceObject = nullptr;
    void (*_commandCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&) = nullptr;
    void (*_stateCallback)(T*, uint8_t*, uint8_t, uint8_t*, uint8_t&) = nullptr;
};
