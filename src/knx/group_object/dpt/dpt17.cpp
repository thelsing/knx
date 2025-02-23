#include "dpt17.h"

#include "dptconvert.h"

Knx::Go_SizeCode Knx::DPT_SceneNumber::size() const
{
    return Go_1_Octet;
}

void Knx::DPT_SceneNumber::encode(uint8_t* data) const
{
    unsigned8ToPayload(data, 0, _value, 0xFF);
}

bool Knx::DPT_SceneNumber::decode(uint8_t* data)
{
    _value = unsigned8FromPayload(data, 0) & 0x3F;
    return true;
}

void Knx::DPT_SceneNumber::value(uint8_t value)
{
    if (value > 63)
        return;

    _value = value;
}
