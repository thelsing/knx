#include "tp_frame.h"
#include "bits.h"

        void TpFrame::printIt() const
        {
            print_ia(source());
            print(" -> ");

            if (isGroupAddress())
                print_ga(destination());
            else
                print_ia(destination());

            print(" [");
            print((flags() & TP_FRAME_FLAG_INVALID) ? 'I' : '_');   // Invalid
            print((flags() & TP_FRAME_FLAG_EXTENDED) ? 'E' : '_');  // Extended
            print((flags() & TP_FRAME_FLAG_REPEATED) ? 'R' : '_');  // Repeat
            print((flags() & TP_FRAME_FLAG_ECHO) ? 'T' : '_');      // Send by me
            print((flags() & TP_FRAME_FLAG_ADDRESSED) ? 'D' : '_'); // Recv for me
            print((flags() & TP_FRAME_FLAG_ACK_NACK) ? 'N' : '_');  // ACK + NACK
            print((flags() & TP_FRAME_FLAG_ACK_BUSY) ? 'B' : '_');  // ACK + BUSY
            print((flags() & TP_FRAME_FLAG_ACK) ? 'A' : '_');       // ACK
            print("] ");
            printHex("( ", data(), size(), false);
            print(")");
        }