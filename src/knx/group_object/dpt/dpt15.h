#pragma once
#include "dpt.h"
namespace Knx
{
    class Dpt15: public Dpt
    {
        enum ReadDirectionValue { LeftToRight = 0, RightToLeft = 1};
        public:
            Dpt15() {};
            Go_SizeCode size() const override;

            void encode(uint8_t* data) const override;
            bool decode(uint8_t* data) override;

            uint32_t accessCode() const;
            void accessCode(const uint32_t value);
            bool detectionError() const;
            void detetionError(const bool value);
            bool permission() const;
            void permission(const bool value);
            ReadDirectionValue readDirection() const;
            void readDirection(const ReadDirectionValue value);
            bool encrypted() const;
            void encrypted(const bool value);
            uint8_t index() const;
            void index(const uint8_t value);
        private:
            uint32_t _accessCode;
            bool _detectionError;
            bool _permission;
            ReadDirectionValue _readDirection;
            bool _encrypted;
            uint8_t _index;
    };

    typedef Dpt15 DPT_Access_Data;
}