#include "application_program_object.h"
#include "bits.h"

ApplicationProgramObject::ApplicationProgramObject(Memory& memory)
    : TableObject(memory)
{

}

void ApplicationProgramObject::readProperty(PropertyID id, uint32_t start, uint32_t& count, uint8_t* data)
{
    switch (id)
    {
        case PID_OBJECT_TYPE:
            pushWord(OT_APPLICATION_PROG, data);
            break;
        case PID_PROG_VERSION:
            pushByteArray(_programVersion, 5, data);
            break;
        case PID_PEI_TYPE:
            pushByte(0x0, data);
            break;
        default:
            TableObject::readProperty(id, start, count, data);
    }
}

void ApplicationProgramObject::writeProperty(PropertyID id, uint8_t start, uint8_t* data, uint8_t count)
{
    switch (id)
    {
        case PID_PROG_VERSION:
            for (uint32_t i = 0; i < 5; i++)
            {
                _programVersion[i] = data[i];
            }
            break;
        default:
            TableObject::writeProperty(id, start, data, count);
    }
}

uint8_t ApplicationProgramObject::propertySize(PropertyID id)
{
    switch (id)
    {
        case PID_PEI_TYPE:
            return 1;
        case PID_OBJECT_TYPE:
            return 2;
        case PID_PROG_VERSION:
            return 5;
    }
    return TableObject::propertySize(id);
}

uint8_t * ApplicationProgramObject::data(uint32_t addr)
{
    return TableObject::data() + addr;
}

uint8_t ApplicationProgramObject::getByte(uint32_t addr)
{
    return *(TableObject::data() + addr);
}

uint16_t ApplicationProgramObject::getWord(uint32_t addr)
{
    return ::getWord(TableObject::data() + addr);
}

uint32_t ApplicationProgramObject::getInt(uint32_t addr)
{
    return ::getInt(TableObject::data() + addr);
}

uint8_t* ApplicationProgramObject::save(uint8_t* buffer)
{
    buffer = pushByteArray(_programVersion, 5, buffer);

    return TableObject::save(buffer);
}

uint8_t* ApplicationProgramObject::restore(uint8_t* buffer)
{
    buffer = popByteArray(_programVersion, 5, buffer);

    return TableObject::restore(buffer);
}

static PropertyDescription _propertyDescriptions[] = 
{
    { PID_OBJECT_TYPE, false, PDT_UNSIGNED_INT, 1, ReadLv3 | WriteLv0 },
    { PID_LOAD_STATE_CONTROL, true, PDT_CONTROL, 1, ReadLv3 | WriteLv3 },
    { PID_TABLE_REFERENCE, false, PDT_UNSIGNED_LONG, 1, ReadLv3 | WriteLv0 },
    { PID_ERROR_CODE, false, PDT_ENUM8, 1, ReadLv3 | WriteLv0 },
    { PID_PEI_TYPE, false, PDT_UNSIGNED_CHAR, 1, ReadLv3 | WriteLv0 },
    { PID_PROG_VERSION, true, PDT_GENERIC_05, 1, ReadLv3 | WriteLv3 },
};
static uint8_t _propertyCount = sizeof(_propertyDescriptions) / sizeof(PropertyDescription);

uint8_t ApplicationProgramObject::propertyCount()
{
    return _propertyCount;
}


PropertyDescription* ApplicationProgramObject::propertyDescriptions()
{
    return _propertyDescriptions;
}

uint16_t ApplicationProgramObject::saveSize()
{
    return TableObject::saveSize() + 5;
}