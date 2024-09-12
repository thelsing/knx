#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt9: public Dpt
    {
        public:
            Dpt9(unsigned short subgroup = 0);
            Dpt9(float value);
            Go_SizeCode size() const override;

            virtual void encode(uint8_t* data) const override;
            virtual void decode(uint8_t* data) override;

            void value(float value);
            float value() const;
            operator float() const;
            Dpt9& operator=(const float value);
        private:
            float _value;
    };
}