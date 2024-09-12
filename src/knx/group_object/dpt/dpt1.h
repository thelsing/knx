#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt1: public Dpt
    {
        public:
            Dpt1();
            Dpt1(bool value);
            Go_SizeCode size() const override;

            virtual void encode(uint8_t* data) const override;
            virtual void decode(uint8_t* data) override;

            void value(bool value);
            bool value();
            operator bool() const;
            Dpt1& operator=(const bool value);
        private:
            bool _value;
    };
}