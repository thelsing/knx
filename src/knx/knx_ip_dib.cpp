#include "knx_ip_dib.h"

DIB::DIB(uint8_t* data) : _data(data)
{}

DescriptionTypeCode DIB::code()
{
    return (DescriptionTypeCode)_data[1];
}


uint8_t DIB::length()
{
    return *_data;
}
