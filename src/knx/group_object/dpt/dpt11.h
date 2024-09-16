#pragma once
#include "dpt.h"
namespace Knx
{
    class DPT_Date: public Dpt
    {
        public:
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            uint8_t day() const;
            void day(uint8_t value);

            uint8_t month() const;
            void month(uint8_t value);

            uint16_t year() const;
            void year(uint16_t value);
        private:
            uint8_t _day;
            uint8_t _month;
            uint16_t _year;
    };


}