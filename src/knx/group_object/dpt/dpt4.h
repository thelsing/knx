#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt4: public DPT<char>
    {
        public:
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    class DPT_Char_ASCII: public Dpt4
    {
            bool decode(uint8_t* data) override;
            void value(char value) override;
    };

    typedef Dpt4  DPT_Char_8859_1;
}