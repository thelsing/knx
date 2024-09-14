#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt4: public ValueDpt<char>
    {
        public:
            Dpt4() {};
            Dpt4(char value) : ValueDpt(value) {}
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;
    };

    class DPT_Char_ASCII: public Dpt4
    {
        public:
                    DPT_Char_ASCII() {};
            DPT_Char_ASCII(char value) : Dpt4(value) {}
            bool decode(uint8_t* data) override;
            void value(char value) override;
    };

    typedef Dpt4  DPT_Char_8859_1;
}