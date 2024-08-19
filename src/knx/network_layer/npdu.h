#pragma once

#include "../util/logger.h"

#include <cstdint>

class CemiFrame;
class TPDU;

class NPDU : public IPrintable
{
        friend class CemiFrame;

    public:
        uint8_t octetCount() const;
        void octetCount(uint8_t value);
        uint8_t length() const;
        uint8_t hopCount() const;
        void hopCount(uint8_t value);
        CemiFrame& frame();
        TPDU& tpdu();
        void printIt() const;

    protected:
        NPDU(uint8_t* data, CemiFrame& frame);

    private:
        uint8_t* _data = 0;
        CemiFrame& _frame;
};