#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt16 : public Dpt
    {
            enum ReadDirectionValue
            {
                LeftToRight = 0,
                RightToLeft = 1
            };

        public:
            Dpt16();
            Dpt16(const char* value);
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            const char* value() const;
            void value(const char* value);

        protected:
            // one character more than the dpt to store \0
            char _value[15];
    };

    typedef Dpt16 DPT_String_8859_1;

    class DPT_String_ASCII : public Dpt16
    {
        public:
            DPT_String_ASCII();
            DPT_String_ASCII(const char* value);
            bool decode(uint8_t* data) override;
    };
} // namespace Knx