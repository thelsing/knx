#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt4: public Dpt
    {
        public:
            Dpt4();
            Dpt4(char value);
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            virtual void value(char value);
            char value() const;
            operator char() const;
            Dpt4& operator=(const char value);
        private:
            char _value;
    };

    class DPT_Char_ASCII: public Dpt4 
    {
        bool decode(uint8_t* data) override;
        void value(char value) override;
    };

    typedef Dpt4  DPT_Char_8859_1;
}