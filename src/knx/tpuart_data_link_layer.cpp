#include "config.h"
#ifdef USE_TP
#pragma GCC optimize("O3")

#include "address_table_object.h"
#include "bits.h"
#include "cemi_frame.h"
#include "device_object.h"
#include "platform.h"
#include "tpuart_data_link_layer.h"

/*
 * A new implementation of the tpuart connection.
 * Author Marco Scholl <develop@marco-scholl.de>
 *
 */

// services Host -> Controller :
// internal commands, device specific
#define U_RESET_REQ 0x01
#define U_STATE_REQ 0x02
#define U_SET_BUSY_REQ 0x03
#define U_QUIT_BUSY_REQ 0x04
#define U_BUSMON_REQ 0x05
#define U_SET_ADDRESS_REQ 0xF1   // different on TP-UART
#define U_L_DATA_OFFSET_REQ 0x08 //-0x0C
#define U_SYSTEM_MODE 0x0D
#define U_STOP_MODE_REQ 0x0E
#define U_EXIT_STOP_MODE_REQ 0x0F
#define U_ACK_REQ 0x10 //-0x17
#define U_ACK_REQ_NACK 0x04
#define U_ACK_REQ_BUSY 0x02
#define U_ACK_REQ_ADRESSED 0x01
#define U_POLLING_STATE_REQ 0xE0

// Only on NCN51xx available
#ifdef NCN5120
#define U_CONFIGURE_REQ 0x18
#define U_CONFIGURE_MARKER_REQ 0x1
#define U_CONFIGURE_CRC_CCITT_REQ 0x2
#define U_CONFIGURE_AUTO_POLLING_REQ 0x4
#define U_SET_REPETITION_REQ 0xF2
#else
#define U_MXRSTCNT 0x24
#endif

// knx transmit data commands
#define U_L_DATA_START_REQ 0x80
#define U_L_DATA_CONT_REQ 0x80 //-0xBF
#define U_L_DATA_END_REQ 0x40  //-0x7F

// serices to host controller

// DLL services (device is transparent)
#define L_DATA_STANDARD_IND 0x90
#define L_DATA_EXTENDED_IND 0x10
#define L_DATA_MASK 0xD3
#define L_POLL_DATA_IND 0xF0

// acknowledge services (device is transparent in bus monitor mode)
#define L_ACKN_IND 0x00
#define L_ACKN_MASK 0x33
#define L_ACKN_BUSY_MASK 0x0C
#define L_ACKN_NACK_MASK 0xC0
#define L_DATA_CON 0x0B
#define L_DATA_CON_MASK 0x7F
#define SUCCESS 0x80

// control services, device specific
#define U_RESET_IND 0x03
#define U_STATE_MASK 0x07
#define U_STATE_IND 0x07
#define SLAVE_COLLISION 0x80
#define RECEIVE_ERROR 0x40
#define TRANSMIT_ERROR 0x20
#define PROTOCOL_ERROR 0x10
#define TEMPERATURE_WARNING 0x08
#define U_FRAME_STATE_IND 0x13
#define U_FRAME_STATE_MASK 0x17
#define PARITY_BIT_ERROR 0x80
#define CHECKSUM_LENGTH_ERROR 0x40
#define TIMING_ERROR 0x20
#define U_CONFIGURE_IND 0x01
#define U_CONFIGURE_MASK 0x83
#define AUTO_ACKNOWLEDGE 0x20
#define AUTO_POLLING 0x10
#define CRC_CCITT 0x80
#define FRAME_END_WITH_MARKER 0x40
#define U_FRAME_END_IND 0xCB
#define U_STOP_MODE_IND 0x2B
#define U_SYSTEM_STAT_IND 0x4B

/*
 * NCN51xx Register handling
 */
// write internal registers
#define U_INT_REG_WR_REQ_WD 0x28
#define U_INT_REG_WR_REQ_ACR0 0x29
#define U_INT_REG_WR_REQ_ACR1 0x2A
#define U_INT_REG_WR_REQ_ASR0 0x2B
// read internal registers
#define U_INT_REG_RD_REQ_WD 0x38
#define U_INT_REG_RD_REQ_ACR0 0x39
#define U_INT_REG_RD_REQ_ACR1 0x3A
#define U_INT_REG_RD_REQ_ASR0 0x3B
// Analog Control Register 0 - Bit values
#define ACR0_FLAG_V20VEN 0x40
#define ACR0_FLAG_DC2EN 0x20
#define ACR0_FLAG_XCLKEN 0x10
#define ACR0_FLAG_TRIGEN 0x08
#define ACR0_FLAG_V20VCLIMIT 0x04

enum
{
    TX_IDLE,
    TX_FRAME
};

enum
{
    // In this state, the system waits for new control commands.
    RX_IDLE,

    // In this state, all bytes are regarded as bytes for a frame.
    RX_FRAME,

    // In this state, all bytes are discarded
    RX_INVALID,

    // Monitoring is still waiting for an ACk
    RX_AWAITING_ACK
};

void printFrame(TpFrame *tpframe)
{
    print(tpframe->humanSource().c_str());
    print(" -> ");
    print(tpframe->humanDestination().c_str());
    print(" [");
    print((tpframe->flags() & TP_FRAME_FLAG_INVALID) ? 'I' : '_');   // Invalid
    print((tpframe->flags() & TP_FRAME_FLAG_EXTENDED) ? 'E' : '_');  // Extended
    print((tpframe->flags() & TP_FRAME_FLAG_REPEATED) ? 'R' : '_');  // Repeat
    print((tpframe->flags() & TP_FRAME_FLAG_ECHO) ? 'T' : '_');      // Send by me
    print((tpframe->flags() & TP_FRAME_FLAG_ADDRESSED) ? 'D' : '_'); // Recv for me
    print((tpframe->flags() & TP_FRAME_FLAG_ACK_NACK) ? 'N' : '_');  // ACK + NACK
    print((tpframe->flags() & TP_FRAME_FLAG_ACK_BUSY) ? 'B' : '_');  // ACK + BUSY
    print((tpframe->flags() & TP_FRAME_FLAG_ACK) ? 'A' : '_');       // ACK
    print("] ");
    printHex("( ", tpframe->data(), tpframe->size(), false);
    print(")");
}

/*
 * Processes all bytes.
 */
void __isr __time_critical_func(TpUartDataLinkLayer::processRx)(bool isr)
{
    if (!isrLock())
        return;

    /*
     * Some platforms support the detection of whether the hardware buffer has overflowed.
     * Theoretically, you could now discard the buffer, but then a valid frame may be lost.
     * Therefore, only one piece of information is output later in the loop and byte processing "tries" to respond to it.
     */
    if (_platform.overflowUart())
        _rxOverflow = true;

    // process data
    while (_platform.uartAvailable())
    {
        processRxByte();
    }

    isrUnlock();
}

/*
 * Processes 1 incoming byte (if available)
 */
void TpUartDataLinkLayer::processRxByte()
{
    int byte = _platform.readUart();

    // RxBuffer empty
    if (byte < 0)
        return;

    /*
     * If I am in RX_INVALID mode
     * and the last byte was processed more than 2ms ago (i.e. pause >2ms)
     * and there are no more bytes in the buffer,
     * then I can discard the INVALID state.
     */
    if (_rxState == RX_INVALID && (millis() - _rxLastTime) > 2 && !_platform.uartAvailable())
    {
        processRxFrameComplete();
        _rxState = RX_IDLE;
    }

    if (_rxState == RX_INVALID)
    {
        /*
         * As soon as a frame has been processed invalidly or an unknown command arrives, the status changes to RX_INVALID.
         * From now on I must assume that there has been a transmission error and the current bytes are invalid.
         * The same applies if a HW overflow is detected.
         *
         * The time of the last frame is 3ms past and there is no more data in the buffer. (Is checked by me)
         * - If the marker mode is active and a U_FRAME_END_IND has been detected correctly. (Checked here)
         *
         * Otherwise this section does nothing and thus discards the invalid bytes
         */
        if (markerMode())
        {
            if (!_rxMarker && byte == U_FRAME_END_IND)
            {
                _rxMarker = true;
            }
            else if (_rxMarker && byte == U_FRAME_END_IND)
            {
                // double byte found so reset marker - no frame end
                _rxMarker = false;
            }
            else if (_rxMarker)
            {
                // frame end found. -> RX_IDLE
                _rxMarker = false;
                _rxState = RX_IDLE;
            }
        }
    }
    else if (_rxState == RX_FRAME)
    {
        processRxFrameByte(byte);
    }
    else if ((byte & L_DATA_MASK) == L_DATA_STANDARD_IND || (byte & L_DATA_MASK) == L_DATA_EXTENDED_IND)
    {
        /*
         * Process a previous frame if still available. This should normally only occur in the bus monitor because an ACK is also being waited for here
         */
        processRxFrameComplete();
        _rxFrame->addByte(byte);

        // Provoke invalid frames for tests
        // if (millis() % 20 == 0)
        //     _rxFrame->addByte(0x1);

        _rxMarker = false;
        _rxState = RX_FRAME;

        /*
         * Here an ack is set inital without Addressed. This is used if an Ack is still set from the previous frame,
         * is set back. This happens if processing is delayed too much (e.g. because no DMA/IRQ is used).
         * The ACK can be sent as often as required because it is only stored in the BCU and is only used / sent when required.
         *
         * Of course, you can only do this if you are not sending yourself, as you do not ACK your own frames. The BCU may ignore this,
         * but I wanted to be on the safe side here.
         */
        if (_txState == TX_IDLE)
        {
            _platform.writeUart(U_ACK_REQ);
        }
    }
    else
    {
        // The commands are evaluated here, if this has already happened.

        if (byte == U_RESET_IND)
        {
            // println("U_RESET_IND");
        }
        else if ((byte & U_STATE_MASK) == U_STATE_IND)
        {
            _tpState |= (byte ^ U_STATE_MASK);
#ifndef NCN5120
            /*
             * Filter "Protocol errors" because this is set on other BCUs such as the Siements when the timing is not correct.
             * Unfortunately, perfect timing is not possible, so this error must be ignored. Also has no known effects.
             */
            _tpState &= 0b11101000;
#endif
        }
        else if ((byte & U_CONFIGURE_MASK) == U_CONFIGURE_IND)
        {
            // println("U_CONFIGURE_IND");
        }
        else if (byte == U_STOP_MODE_IND)
        {
            // println("U_STOP_MODE_IND");
        }
        else if ((byte & L_ACKN_MASK) == L_ACKN_IND)
        {
            /*
             * If a frame has not yet been closed and an Ack comes in.
             * then set the ACK.
             */
            if (_rxFrame->size() > 0)
            {
                if (!(byte & L_ACKN_BUSY_MASK))
                    _rxFrame->addFlags(TP_FRAME_FLAG_ACK_BUSY);

                if (!(byte & L_ACKN_NACK_MASK))
                    _rxFrame->addFlags(TP_FRAME_FLAG_ACK_NACK);

                _rxFrame->addFlags(TP_FRAME_FLAG_ACK);
                processRxFrameComplete();
            }
            // println("L_ACKN_IND");
        }
        else if ((byte & L_DATA_CON_MASK) == L_DATA_CON)
        {
            if (_txState == TX_FRAME)
            {
                const bool success = ((byte ^ L_DATA_CON_MASK) >> 7);
                processTxFrameComplete(success);
            }
            else
            {
                // This byte was not expected because nothing was sent.
                _rxUnkownControlCounter++;
                _rxState = RX_INVALID;
                // println("L_DATA_CON");
            }
        }
        else if (byte == L_POLL_DATA_IND)
        {
            // println("L_POLL_DATA_IND");
        }
        else if ((byte & U_FRAME_STATE_MASK) == U_FRAME_STATE_IND)
        {
            // println("U_FRAME_STATE_IND");
        }
        else
        {
            _rxUnkownControlCounter++;
            // print("Unknown Controlbyte: ");
            // println(byte, HEX);
            _rxState = RX_INVALID;
        }
    }

    _rxLastTime = millis();
}

/*
 * Process incoming byte of a frame
 */
void TpUartDataLinkLayer::processRxFrameByte(uint8_t byte)
{
    /*
     * If the maker is active, the first U_FRAME_END_IND must be ignored and a subsequent byte must be waited for.
     * The subsequent byte is therefore decisive for how this byte is to be evaluated.
     */
    if (markerMode() && (byte == U_FRAME_END_IND && !_rxMarker))
    {
        _rxMarker = true;
    }

    /*
     * If the previous byte was a U_FRAME_END_IND and the new byte is a U_FRAME_STATE_IND,
     * then the reception is cleanly completed and the frame can be processed.
     */
    else if (_rxMarker && (byte & U_FRAME_STATE_MASK) == U_FRAME_STATE_IND)
    {
        _rxMarker = false;
        processRxFrameComplete();

        /*
         * Set the status to RX_IDLE, as the marker ensures,
         * that the frame has been processed successfully. Subsequent bytes are therefore clean again Control commands,
         * even if the frame was discarded due to an invalid checksum (which would mean RX_INVAID)
         */
        _rxState = RX_IDLE;
    }

    /*
     * This is a hypothetical case in which the frames are sent without markers even though marker mode is active.
     * Here the current frame is processed and RX_INVALID is set, as the current byte is not processed.
     * This case can occur if the marker mode is not supported by the TPUart (NCN51xx feature) but has been activated.
     */
    else if (markerMode() && _rxFrame->isFull())
    {
        processRxFrameComplete();
        /*
         * RX_INVALID because theoretically the frame could have been processed as valid.
         * However, since the current byte has already been "started" to be processed, it is missing in the processing chain
         * and therefore the subsequent bytes cannot be used.
         */
        _rxState = RX_INVALID;
    }

    /*
     * If marker mode is active, the byte should be processed normally.
     * If marker mode is active, a U_FRAME_END_IND byte may only be processed if the previous byte was also a U_FRAME_END_IND.
     */
    else if (!markerMode() || byte != U_FRAME_END_IND || (byte == U_FRAME_END_IND && _rxMarker))
    {
        // Reset the marker if active
        _rxMarker = false;
        // Accept the byte
        _rxFrame->addByte(byte);

        // If the bus monitor has been started, no processing takes place - i.e. no ACKing
        if (!_monitoring)
        {
            // If more than 7 bytes are available, you can check whether the frame is intended for "me".
            if (_rxFrame->size() == 7)
            {
                // Check whether I am responsible for the frame
                TPAckType ack = _cb.isAckRequired(_rxFrame->destination(), _rxFrame->isGroupAddress());
                if (_forceAck || ack)
                {
                    /*
                     * Save the responsibility that this frame is to be processed further.
                     * Since there is no extra function apart from the isAckRequired, this is initially treated the same.
                     * A later differentiation (possibly for router mode) must then be looked at.
                     */

                    _rxFrame->addFlags(TP_FRAME_FLAG_ADDRESSED);

                    // Of course, this is only allowed if I am not sending myself, as you cannot ACK your own frames
                    if (_txState == TX_IDLE)
                    {
                        // Save that Acking should take place
                        _rxFrame->addFlags(TP_FRAME_FLAG_ACK);

                        // and in the TPUart so that it can send the ACK
                        _platform.writeUart(U_ACK_REQ | ack);
                    }
                }
            }

#ifdef USE_TP_RX_QUEUE
            // Now check whether the RxQueue still has space for Frame + Size (2) + Flags(1)
            if (_rxFrame->size() == 8 && (_rxFrame->flags() & TP_FRAME_FLAG_ADDRESSED))
            {
                if (availableInRxQueue() < (_rxFrame->size() + 3))
                {
                    // Only if I am not sending myself
                    if (_txState == TX_IDLE)
                    {
                        _platform.writeUart(U_ACK_REQ | U_ACK_REQ_ADRESSED | U_ACK_REQ_BUSY);
                    }
                }
            }
#endif
        }
    }

    /*
     * If no marker mode is active, the frame must be checked to see if it is complete.
     * isFull checks here whether the maxSize or the length specification of the frame has been exceeded!
     * In both cases, the frame must be processed.
     */
    if (!markerMode() && (_rxFrame->isFull()))
    {
        processRxFrameComplete();
    }
}

/*
 * Processes the current frame and checks whether it is complete and valid (checksum).
 * If a frame is complete and valid, it is placed in the queue if it is intended for "me" and the mode is RX_IDLE again.
 * Otherwise the frame is discarded as invalid and the status is RX_INVALID, as it is not guaranteed that subsequent bytes are control codes again.
 * Exception in marker mode, here the status RX_INVALID is changed directly back to RX_IDLE at another point because
 * it is then ensured that the frame has been broken at TP level.
 */
void TpUartDataLinkLayer::processRxFrameComplete()
{
    // If no frame is currently being edited, then cancel
    if (!_rxFrame->size())
        return;

    // Is the frame complete and valid
    if (_rxFrame->isValid())
    {
        // When a frame has been sent
        if (_txState == TX_FRAME)
        {
            // check whether the receive corresponds to this: comparison of the source address and destination address and start byte without taking the retry bit into account
            if(!((_rxFrame->data(0) ^ _txFrame->data(0)) & ~0x20) && _rxFrame->destination() == _txFrame->destination() && _rxFrame->source() == _txFrame->source())
            {
                // and mark this accordingly
                // println("MATCH");
                _rxFrame->addFlags(TP_FRAME_FLAG_ECHO);
            }
            // Now wait for the L_DATA_CON
        }
        // if the frame is for me or i am in busmonitor mode then i want to process it further
        if (_rxFrame->flags() & TP_FRAME_FLAG_ADDRESSED || _monitoring)
        {
            /*
             * In bus monitor mode, you still have to wait for an Ack.
             * Therefore, the status is changed here and jumps back before the real completion.
             * As soon as another call is made (regardless of whether or not the frame has been acked), the frame is closed.
             */
            if (_monitoring && _rxState != RX_AWAITING_ACK)
            {
                _rxState = RX_AWAITING_ACK;
                return;
            }
            _rxProcessdFrameCounter++;
        }
        else
        {
            // Otherwise, discard the package and release the memory -> as it is not packed into the queue
            _rxIgnoredFrameCounter++;
        }
        // And ready for control codes again
        _rxState = RX_IDLE;
    }
    else
    {
        /*
         * If the frame is incomplete or invalid then switch to RX_INVALID mode as I cannot distinguish,
         * whether it is a TPBus error or a UART error or a Timming error.
         */
        _rxInvalidFrameCounter++;
        _rxFrame->addFlags(TP_FRAME_FLAG_INVALID);
        _rxState = RX_INVALID; // ignore bytes
    }

#ifdef USE_TP_RX_QUEUE
    pushRxFrameQueue();
#else
    processRxFrame(_rxFrame);
#endif

    // resets the current frame pointer
    _rxFrame->reset();
}

void TpUartDataLinkLayer::clearTxFrame()
{
    if (_txFrame != nullptr)
    {
        delete _txFrame;
        _txFrame = nullptr;
    }
}

void TpUartDataLinkLayer::clearTxFrameQueue()
{
}

void TpUartDataLinkLayer::processTxFrameComplete(bool success)
{
    uint8_t *cemiData = _txFrame->cemiData();
    CemiFrame cemiFrame(cemiData, _txFrame->cemiSize());
    dataConReceived(cemiFrame, success);
    free(cemiData);
    clearTxFrame();
    _txProcessdFrameCounter++;
    _txState = TX_IDLE;
}

/*
 * Puts the frame to be sent into a queue, as the TpUart may not yet be ready to send.
 */
void TpUartDataLinkLayer::pushTxFrameQueue(TpFrame *tpFrame)
{
    knx_tx_queue_entry_t *entry = new knx_tx_queue_entry_t(tpFrame);

    if (_txFrameQueue.back == nullptr)
    {
        _txFrameQueue.front = _txFrameQueue.back = entry;
    }
    else
    {
        _txFrameQueue.back->next = entry;
        _txFrameQueue.back = entry;
    }

    _txQueueCount++;
}

void TpUartDataLinkLayer::setRepetitions(uint8_t nack, uint8_t busy)
{
    _repetitions = (nack & 0b111) | ((busy & 0b111) << 4);
}

// Alias
void TpUartDataLinkLayer::setFrameRepetition(uint8_t nack, uint8_t busy)
{
    setRepetitions(nack, busy);
}

bool TpUartDataLinkLayer::sendFrame(CemiFrame &cemiFrame)
{
    _txFrameCounter++;

    if (!_connected || _monitoring || _txQueueCount > MAX_TX_QUEUE)
    {
        if (_txQueueCount > MAX_TX_QUEUE)
        {
            println("Ignore frame because transmit queue is full!");
        }

        dataConReceived(cemiFrame, false);
        return false;
    }

    TpFrame *tpFrame = new TpFrame(cemiFrame);
    // printHex("  TP>: ", tpFrame->data(), tpFrame->size());
    pushTxFrameQueue(tpFrame);
    return true;
}

/*
 * The status should be queried regularly to detect a disconnect of the TPUart and its status.
 * In addition, the current config or mode should be transmitted regularly so that after a disconnect,
 * the TPUart is in the correct state.
 */
void TpUartDataLinkLayer::requestState(bool force /* = false */)
{
    if (!force)
    {
        if (!(_rxState == RX_IDLE || _rxState == RX_INVALID))
            return;

        // Only 1x per second
        if ((millis() - _lastStateRequest) < 1000)
            return;
    }

    // println("requestState");

    // Send configuration or mode
    if (_monitoring)
        _platform.writeUart(U_BUSMON_REQ);
    else
        requestConfig();

    // Question status on - if monitoring inactive
    if (!_monitoring)
        _platform.writeUart(U_STATE_REQ);

    _lastStateRequest = millis();
}

/*
 * Sends the current config to the chip
 */
void TpUartDataLinkLayer::requestConfig()
{
    // println("requestConfig");
#ifdef NCN5120
    if (markerMode())
        _platform.writeUart(U_CONFIGURE_REQ | U_CONFIGURE_MARKER_REQ);
#endif

    // Deviating Config
    if (_repetitions != 0b00110011)
    {
#ifdef NCN5120
        _platform.writeUart(U_SET_REPETITION_REQ);
        _platform.writeUart(_repetitions);
        _platform.writeUart(0x0); // dummy, see NCN5120 datasheet
        _platform.writeUart(0x0); // dummy, see NCN5120 datasheet
#else
        _platform.writeUart(U_MXRSTCNT);
        _platform.writeUart(((_repetitions & 0xF0) << 1) || (_repetitions & 0x0F));
#endif
    }
}

/*
 * A simplified lock mechanism that only works on the same core.
 * Perfect for ISR
 */
bool TpUartDataLinkLayer::isrLock(bool blocking /* = false */)
{
    if (blocking)
        while (_rxProcessing)
            ;
    else if (_rxProcessing)
        return false;

    _rxProcessing = true;
    return true;
}

void TpUartDataLinkLayer::isrUnlock()
{
    _rxProcessing = false;
}

void TpUartDataLinkLayer::clearUartBuffer()
{
    // Clear rx queue
    while (_platform.uartAvailable())
        _platform.readUart();
}

void TpUartDataLinkLayer::connected(bool state /* = true */)
{
    if (state)
        println("TP is connected");
    else
        println("TP is disconnected");

    _connected = state;
}

void TpUartDataLinkLayer::resetStats()
{
    _rxProcessdFrameCounter = 0;
    _rxIgnoredFrameCounter = 0;
    _rxInvalidFrameCounter = 0;
    _rxInvalidFrameCounter = 0;
    _rxUnkownControlCounter = 0;
    _txFrameCounter = 0;
    _txProcessdFrameCounter = 0;
}

bool TpUartDataLinkLayer::reset()
{
    // println("Reset TP");
    if (!_initialized)
    {
        _platform.setupUart();
        _initialized = true;
    }

    // Wait for isr & block isr
    isrLock(true);

    // Reset
    resetStats();
    clearTxFrame();
    clearTxFrameQueue();

    if (_rxFrame != nullptr)
    {
        _rxFrame->reset();
    }
    _rxState = RX_IDLE;
    _connected = false;
    _stopped = false;
    _monitoring = false;
    _rxLastTime = 0;

    clearUartBuffer();

    _platform.writeUart(U_RESET_REQ);
    bool success = false;

    const uint32_t start = millis();
    // During startup answer took up to 2ms and normal 1ms
    do
    {
        const int byte = _platform.readUart();
        if (byte == -1)
            continue; // empty

        if (byte & U_RESET_IND)
        {
            success = true;
            break; // next run for U_CONFIGURE_IND
        }

    } while (!((millis() - start) >= 10));

    connected(success);
    if (success)
    {
        _lastStateRequest = 0; // Force
        requestState(true);
        _rxLastTime = millis();
    }

    isrUnlock();
    return success;
}

void TpUartDataLinkLayer::forceAck(bool state)
{
    _forceAck = true;
}

void TpUartDataLinkLayer::stop(bool state)
{
    if (!_initialized)
        return;

    if (state && !_stopped)
        _platform.writeUart(U_STOP_MODE_REQ);
    else if (!state && _stopped)
        _platform.writeUart(U_EXIT_STOP_MODE_REQ);

    _stopped = state;
}

void TpUartDataLinkLayer::requestBusy(bool state)
{
    if (state && !_busy)
        _platform.writeUart(U_SET_BUSY_REQ);
    else if (!state && _busy)
        _platform.writeUart(U_QUIT_BUSY_REQ);

    _busy = state;
}

void TpUartDataLinkLayer::monitor()
{
    if (!_initialized || _monitoring)
        return;

    // println("busmonitor");
    _monitoring = true;
    _platform.writeUart(U_BUSMON_REQ);
    resetStats();
}

void TpUartDataLinkLayer::enabled(bool value)
{
    // After an unusual device restart, perform a reset, as the TPUart may still be in an incorrect state.
    if (!_initialized)
        reset();

    stop(!value);
}

bool TpUartDataLinkLayer::enabled() const
{
    return _initialized && _connected;
}

/*
 * If a TxFrame has been sent, a confirmation for the transmission is expected.
 * However, if there was an invalid frame or bus disconnect, the confirmation is not received and the STack is stuck in the TX_FRAME.
 * The wait must therefore be ended after a short waiting time.
 */
void TpUartDataLinkLayer::clearOutdatedTxFrame()
{
    if (_txState == TX_FRAME && (millis() - _txLastTime) > 1000)
        processTxFrameComplete(false);
}

/*
 * Here the outgoing frames are taken from the queue and sent.
 * This only happens one at a time, as after each frame it is necessary to wait until the frame has come in again and the L_DATA_CON comes in.
 *
 */
void TpUartDataLinkLayer::processTxQueue()
{
    if (_txState != TX_IDLE)
        return;

    if (_txFrameQueue.front != nullptr)
    {
        knx_tx_queue_entry_t *entry = _txFrameQueue.front;
        _txFrameQueue.front = entry->next;

        if (_txFrameQueue.front == nullptr)
        {
            _txFrameQueue.back = nullptr;
        }

        _txQueueCount--;

        clearTxFrame();

        // use frame from queue and delete queue entry
        _txFrame = entry->frame;
        delete entry;

        _txState = TX_FRAME;
        _txLastTime = millis();

#ifdef DEBUG_TP_FRAMES
        print("Outbound: ");
        printFrame(_txFrame);
        println();
#endif

        processTxFrameBytes();
    }
}

/*
 * Check whether I have not received any data for too long and set the status to not connected.
 * In normal mode, the status is requested every second. A short time can therefore be selected here.
 * In monitoring mode there are actual frames, so a longer time is used here.
 * Nevertheless, there are suspected disconnects with larger data volumes, so the RxQueue is also taken into account.
 */
void TpUartDataLinkLayer::checkConnected()
{
    if (!isrLock())
        return;

    const uint32_t current = millis();

    if (_connected)
    {
        // 5000 instead 3000 because siemens tpuart
        const uint32_t timeout = _monitoring ? 10000 : 5000;

        if ((current - _rxLastTime) > timeout)
        {
            connected(false);
        }
    }
    else
    {
        if (_rxLastTime > 0 && (current - _rxLastTime) < 1000)
            connected();
    }

    isrUnlock();
}

void TpUartDataLinkLayer::loop()
{
    if (!_initialized)
        return;

    /*
     * If an overflow has been detected, change to RX_INVALID.
     * However, this only applies in the loop and not in ISR. But when using ISR and DMA, this should never happen.
     */
    if (_rxOverflow)
    {
        println("TPUart overflow detected!");
        _rxOverflow = false;
        _rxState = RX_INVALID;
    }

    if (_tpState)
    {
        print("TPUart state error: ");
        println(_tpState, 2);
        _tpState = 0;
    }

    processRx();
#ifdef USE_TP_RX_QUEUE
    processRxQueue();
#endif

    requestState();
    clearOutdatedTxFrame();
    processTxQueue();
    checkConnected();
}

void TpUartDataLinkLayer::rxFrameReceived(TpFrame *tpFrame)
{
    uint8_t *cemiData = tpFrame->cemiData();
    CemiFrame cemiFrame(cemiData, tpFrame->cemiSize());
    // printHex("  TP<: ", tpFrame->data(), tpFrame->size());
    // printHex("  CEMI<: ", cemiFrame.data(), cemiFrame.dataLength());

#ifdef KNX_ACTIVITYCALLBACK
    if (_dllcb)
        _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_RECV << KNX_ACTIVITYCALLBACK_DIR));
#endif

    frameReceived(cemiFrame);
    free(cemiData);
}

DptMedium TpUartDataLinkLayer::mediumType() const
{
    return DptMedium::KNX_TP1;
}

/*
 * This can be used to switch the power supply to the V20V (VCC2)
 */
#ifdef NCN5120
void TpUartDataLinkLayer::powerControl(bool state)
{
    _platform.writeUart(U_INT_REG_WR_REQ_ACR0);
    if (state)
        _platform.writeUart(ACR0_FLAG_DC2EN | ACR0_FLAG_V20VEN | ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT);
    else
        _platform.writeUart(ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT);
}
#endif

bool TpUartDataLinkLayer::processTxFrameBytes()
{
    // println("processTxFrameBytes");

    /*
     * Each frame must be introduced with a U_L_DATA_START_REQ and each subsequent byte with a further position byte (6bit).
     * Since the position byte consists of the U_L_DATA_START_REQ + position and we start with 0 anyway, a further distinction is not necessary.
     * distinction is not necessary.
     *
     * However, the last byte (checksum) uses the U_L_DATA_END_REQ + position!
     * In addition, there is another special feature for extended frames up to 263 bytes long, the 6 bits are no longer sufficient.
     * Here a U_L_DATA_OFFSET_REQ + Position (3bit) must be prefixed. This means that 9 bits are available for the position.
     */
    for (uint16_t i = 0; i < _txFrame->size(); i++)
    {
        uint8_t offset = (i >> 6);
        uint8_t position = (i & 0x3F);

        if (offset)
        {
            // position++;
            _platform.writeUart(U_L_DATA_OFFSET_REQ | offset);
        }

        if (i == (_txFrame->size() - 1)) // Last bytes (checksum)
            _platform.writeUart(U_L_DATA_END_REQ | position);
        else
            _platform.writeUart(U_L_DATA_START_REQ | position);

        _platform.writeUart(_txFrame->data(i));
    }

#ifdef KNX_ACTIVITYCALLBACK
    if (_dllcb)
        _dllcb->activity((_netIndex << KNX_ACTIVITYCALLBACK_NET) | (KNX_ACTIVITYCALLBACK_DIR_SEND << KNX_ACTIVITYCALLBACK_DIR));
#endif

    return true;
}

TpUartDataLinkLayer::TpUartDataLinkLayer(DeviceObject &devObj,
                                         NetworkLayerEntity &netLayerEntity,
                                         Platform &platform,
                                         BusAccessUnit& busAccessUnit,
                                         ITpUartCallBacks &cb,
                                         DataLinkLayerCallbacks *dllcb)
    : DataLinkLayer(devObj, netLayerEntity, platform, busAccessUnit),
      _cb(cb),
      _dllcb(dllcb)
{
    _rxFrame = new TpFrame(MAX_KNX_TELEGRAM_SIZE);
}

/*
 * Returns the number of frames that could not be processed.
 */
uint32_t TpUartDataLinkLayer::getRxInvalidFrameCounter()
{
    return _rxInvalidFrameCounter;
}

/*
 * Returns the number of frames that are valid and intended for the device
 */
uint32_t TpUartDataLinkLayer::getRxProcessdFrameCounter()
{
    return _rxProcessdFrameCounter;
}

/*
 * Returns the number of frames that are valid but not intended for the device
 */
uint32_t TpUartDataLinkLayer::getRxIgnoredFrameCounter()
{
    return _rxIgnoredFrameCounter;
}

/*
 * Returns the number of control bytes counted that were not recognized
 */
uint32_t TpUartDataLinkLayer::getRxUnknownControlCounter()
{
    return _rxUnkownControlCounter;
}

/*
 * Returns the number of frames sent
 */
uint32_t TpUartDataLinkLayer::getTxFrameCounter()
{
    return _txFrameCounter;
}
/*
 * Returns the number of frames sent
 */
uint32_t TpUartDataLinkLayer::getTxProcessedFrameCounter()
{
    return _txProcessdFrameCounter;
}

bool TpUartDataLinkLayer::isConnected()
{
    return _connected;
}

bool TpUartDataLinkLayer::isStopped()
{
    return _stopped;
}

bool TpUartDataLinkLayer::isBusy()
{
    return _busy;
}

bool TpUartDataLinkLayer::isMonitoring()
{
    return _monitoring;
}

bool TpUartDataLinkLayer::markerMode()
{
    if (_monitoring)
        return false;

#ifdef NCN5120
        // return true;
#endif

    return false;
}

void TpUartDataLinkLayer::processRxFrame(TpFrame *tpFrame)
{
    if (_monitoring)
    {
        print("Monitor:  ");
        printFrame(tpFrame);
        println();
    }
    else if (tpFrame->flags() & TP_FRAME_FLAG_INVALID)
    {
        print("\x1B[");
        print(31);
        print("m");
        print("Invalid:  ");
        printFrame(tpFrame);
        print("\x1B[");
        print(0);
        println("m");
    }
    else if (tpFrame->flags() & TP_FRAME_FLAG_ADDRESSED)
    {
#ifdef DEBUG_TP_FRAMES
        print("Inbound:  ");
        printFrame(tpFrame);
        println();
#endif
        if (!(tpFrame->flags() & TP_FRAME_FLAG_ECHO))
            rxFrameReceived(tpFrame);
    }
}

#ifdef USE_TP_RX_QUEUE
/*
 * This method allows the processing of the incoming bytes to be handled additionally via an interrupt (ISR).
 * The prerequisite is that the interrupt runs on the same core as the knx.loop!
 *
 * With an RP2040 where the ISR is also locked when a block is erased,
 * processing can be caught up between the erases. This significantly minimizes the risk of frame losses.
 */
void __isr __time_critical_func(TpUartDataLinkLayer::processRxISR)()
{
    processRx(true);
}

/*
 * Puts the received frame into a queue. This queue is necessary,
 * because a frame can optionally be received via an ISR and processing must still take place normally in the knx.loop.
 * In addition, this queue is statically preallocated, as no malloc etc. can be made in an ISR.
 */
void TpUartDataLinkLayer::pushRxFrameQueue()
{
    if (availableInRxQueue() < (_rxFrame->size() + 3))
        return;

    // Payloadsize (2 byte)
    pushByteToRxQueue(_rxFrame->size() & 0xFF);
    pushByteToRxQueue(_rxFrame->size() >> 8);
    // Paylodflags (1 byte)
    pushByteToRxQueue(_rxFrame->flags());

    for (size_t i = 0; i < _rxFrame->size(); i++)
    {
        pushByteToRxQueue(_rxFrame->data(i));
    }

    asm volatile("" ::: "memory");
    _rxBufferCount++;
}

void TpUartDataLinkLayer::processRxQueue()
{
    if (!isrLock())
        return;

    while (_rxBufferCount)
    {
        const uint16_t size = pullByteFromRxQueue() + (pullByteFromRxQueue() << 8);
        TpFrame tpFrame = TpFrame(size);
        tpFrame.addFlags(pullByteFromRxQueue());

        for (uint16_t i = 0; i < size; i++)
            tpFrame.addByte(pullByteFromRxQueue());

        processRxFrame(&tpFrame);
        asm volatile("" ::: "memory");
        _rxBufferCount--;
    }

    isrUnlock();
}

void TpUartDataLinkLayer::pushByteToRxQueue(uint8_t byte)
{
    _rxBuffer[_rxBufferFront] = byte;
    _rxBufferFront = (_rxBufferFront + 1) % (MAX_RX_QUEUE_BYTES);
}

uint8_t TpUartDataLinkLayer::pullByteFromRxQueue()
{
    uint8_t byte = _rxBuffer[_rxBufferRear];
    _rxBufferRear = (_rxBufferRear + 1) % (MAX_RX_QUEUE_BYTES);
    return byte;
}

uint16_t TpUartDataLinkLayer::availableInRxQueue()
{
    return ((_rxBufferFront == _rxBufferRear) ? (MAX_RX_QUEUE_BYTES) : ((((MAX_RX_QUEUE_BYTES) - _rxBufferFront) + _rxBufferRear) % (MAX_RX_QUEUE_BYTES))) - 1;
}
#endif

#endif