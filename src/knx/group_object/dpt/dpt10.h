#pragma once
#include "dpt.h"
namespace Knx
{
    class DPT_TimeOfDay: public Dpt
    {
        public:
            enum DayOfWeekValue
            {
                NoDay = 0,
                Monday = 1,
                Tuesday = 2,
                Wednesday = 3,
                Thursday = 4,
                Friday = 5,
                Saturday = 6,
                Sunday = 7
            };


            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            DayOfWeekValue day() const;
            void day(DayOfWeekValue value);

            uint8_t hours() const;
            void hours(uint8_t value);

            uint8_t minutes() const;
            void minutes(uint8_t value);

            uint8_t seconds() const;
            void seconds(uint8_t value);
        private:
            DayOfWeekValue _dow;
            uint8_t _hours;
            uint8_t _minutes;
            uint8_t _seconds;
    };


}