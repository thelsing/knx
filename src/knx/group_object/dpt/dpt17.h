#pragma once
#include "dpt.h"
namespace Knx
{
    class DPT_SceneNumber : public ValueDpt<uint8_t>
    {
            enum ReadDirectionValue
            {
                LeftToRight = 0,
                RightToLeft = 1
            };

        public:
            DPT_SceneNumber();
            DPT_SceneNumber(const char* value);
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            void value(uint8_t value) override;
    };
} // namespace Knx