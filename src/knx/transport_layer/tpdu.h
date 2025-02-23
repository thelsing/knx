#pragma once

#include "../knx_types.h"
#include "../util/logger.h"

#include <cstdint>

namespace Knx
{
    class CemiFrame;
    class APDU;

    class TPDU : public IPrintable
    {
            friend class CemiFrame;

        public:
            TpduType type() const;
            void type(TpduType type);

            bool numbered() const;
            void numbered(bool value);

            bool control() const;
            void control(bool value);

            uint8_t sequenceNumber() const;
            void sequenceNumber(uint8_t value);

            APDU& apdu();

            CemiFrame& frame();
            void printIt() const;

        protected:
            TPDU(uint8_t* data, CemiFrame& frame);

        private:
            uint8_t* _data = 0;
            CemiFrame& _frame;
    };
} // namespace Knx