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
 * To avoid misunderstandings (also for myself), this is in German, at least for the time being.
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
    // In diesem Zustand wird auf neue Steuerbefehle gewartet.
    RX_IDLE,

    // In diesem Zustand werden alle Bytes als Bytes für ein Frame betrachtet.
    RX_FRAME,

    // In diesem Zustand werdem alle Bytes verworfen
    RX_INVALID,

    // Im Monitoring wird noch auf ein ACk gewartet
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
 * Verarbeitet alle Bytes.
 */
void __isr __time_critical_func(TpUartDataLinkLayer::processRx)(bool isr)
{
    if (!isrLock())
        return;

    /*
     * Manche Platformen untersützen die Erkennung, ob der Hardwarebuffer übergelaufen ist.
     * Theoretisch könnte man nun den Buffer verwerfen, aber dann geht ggf. noch ein gültiges Frame verloren.
     * Daher wird später im loop nur eine Info ausgegben und im Byteprocessing wird "versucht" noch darauf einzugehen.
     */
    if (_platform.overflowUart())
        _rxOverflow = true;

    // verarbeiten daten
    while (_platform.uartAvailable())
    {
        processRxByte();
    }

    isrUnlock();
}

/*
 * Verarbeitet 1 eigehendes Byte (wenn vorhanden)
 */
void TpUartDataLinkLayer::processRxByte()
{
    int byte = _platform.readUart();

    // RxBuffer empty
    if (byte < 0)
        return;

    /*
     * Wenn ich im RX_INVALID Modus bin
     *   und das letzte Byte vor mehr als 2ms verarbeitet wurde (also pause >2ms)
     *   und keine weiteren Bytes in der Buffer vorliegen,
     * dann kann ich den INVALID State verwerfen.
     */
    if (_rxState == RX_INVALID && (millis() - _rxLastTime) > 2 && !_platform.uartAvailable())
    {
        processRxFrameComplete();
        _rxState = RX_IDLE;
    }

    if (_rxState == RX_INVALID)
    {
        /*
         * Sobald ein Frame ungültig verarbeitet wurde oder ein unbekanntes Kommando eintrifft, wechselt der Zustand in RX_INVALID.
         * Ab jetzt muss ich davon ausgehen, dass einen Übertragungsfehler gegeben hat und die aktuellen Bytes ungültig sind.
         * Gleiches gilt auch wenn ein HW Overflow erkannt wurde.
         *
         * - Die Zeit des letzten Frames 3ms vorbei ist und keine Daten mehr im Buffer sind. (Wird übermir geprüft)
         * - Wenn der Markermodus aktiv ist und ein U_FRAME_END_IND sauber erkannt wurde. (Wird hier geprüft)
         *
         * Ansonsten macht dieser Abschnitt nichts und verwirrft damit die ungültigen Bytes
         */
        if (markerMode())
        {
            if (!_rxMarker && byte == U_FRAME_END_IND)
            {
                _rxMarker = true;
            }
            else if (_rxMarker && byte == U_FRAME_END_IND)
            {
                // doppeltes byte gefunden also marker zurück setzten - kein Frameende
                _rxMarker = false;
            }
            else if (_rxMarker)
            {
                // frame ende gefunden. -> RX_IDLE
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
         * Verarbeite ein vorheriges Frame falls noch vorhanden. Das dürfte normal nur im Busmonitor auftreten, weil hier noch auch ein ACK gewartet wird
         */
        processRxFrameComplete();
        _rxFrame->addByte(byte);

        // Provoziere ungültige Frames für Tests
        // if (millis() % 20 == 0)
        //     _rxFrame->addByte(0x1);

        _rxMarker = false;
        _rxState = RX_FRAME;

        /*
         * Hier wird inital ein Ack ohne Addressed gesetzt. Das dient dazu, falls noch ein Ack vom vorherigen Frame gesetzt ist,
         * zurück gesetzt wird. Das passiert wenn die Verarbeitung zu stark verzögert ist (z.B. weil kein DMA/IRQ genutzt wird).
         * Das ACK kann beliebig oft gesendet werden, weil es in der BCU nur gespeichert wird und erst bei bedarf genutzt / gesendet wird.
         *
         * Das darf man natürlich nur wenn ich nicht gerade selber sende, da man eigene Frames nicht ACKt. Ggf ignoriert die BCU dies,
         * aber ich wollte hier auf sicher gehen.
         */
        if (_txState == TX_IDLE)
        {
            _platform.writeUart(U_ACK_REQ);
        }
    }
    else
    {
        // Hier werden die Commands ausgewertet, falls das noch schon passiert ist.

        if (byte == U_RESET_IND)
        {
            // println("U_RESET_IND");
        }
        else if ((byte & U_STATE_MASK) == U_STATE_IND)
        {
            _tpState |= (byte ^ U_STATE_MASK);
#ifndef NCN5120
            /*
             * Filtere "Protocol errors" weil auf anderen BCU wie der Siements dies gesetzt, when das timing nicht stimmt.
             * Leider ist kein perfektes Timing möglich, so dass dieser Fehler ignoriert werden muss. Hat auch keine bekannte Auswirkungen.
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
             * Wenn ein Frame noch nicht geschlossen wurde und ein Ack rein kommt.
             * dann setze noch das ACK.
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
                // Dieses Byte wurde nicht erwartet, da garnichts gesendet wurde.
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
 * Verarbeite eigehendes Byte eines Frames
 */
void TpUartDataLinkLayer::processRxFrameByte(uint8_t byte)
{
    /*
     * Bei aktivem Maker muss das erste U_FRAME_END_IND ignoriert und auf ein Folgebyte gewartet werden.
     * Das Folgebyte ist also ausschlaggebend wie dieses Byte zu bewerten ist.
     */
    if (markerMode() && (byte == U_FRAME_END_IND && !_rxMarker))
    {
        _rxMarker = true;
    }

    /*
     * Wenn das vorherige Byte ein U_FRAME_END_IND war, und das neue Byte ein U_FRAME_STATE_IND ist,
     * dann ist der Empfang sauber abgeschlossen und das Frame kann verarbeitet werden.
     */
    else if (_rxMarker && (byte & U_FRAME_STATE_MASK) == U_FRAME_STATE_IND)
    {
        _rxMarker = false;
        processRxFrameComplete();

        /*
         * Setze den Zustand auf RX_IDLE, da durch den Marker sichergestellt ist,
         * dass das Frame erfolgreich verarbeitet wurde. Nachfolgende Bytes sind also wieder sauber Steuerbefehle,
         * selbst wenn das Frame aufgrund einer ungültigen Checksumme verworfen wurde (was RX_INVAID) bedeuten würde
         */
        _rxState = RX_IDLE;
    }

    /*
     * Dies ist ein hypotetischer Fall, dass die Frames ohne Marker kommen, obwohl der MarkerModus aktiv ist.
     * Hier wird der aktuelle Frame abgearbeitet und RX_INVALID gesetzt, da das aktuelle Byte hierbei nicht bearbeitet wird.
     * Dieser Fall kann eintreffen wenn der Marker Modus von der TPUart nicht unterstützt wird (NCN51xx Feature) aber aktiviert wurde.
     */
    else if (markerMode() && _rxFrame->isFull())
    {
        processRxFrameComplete();
        /*
         * RX_INVALID weil theoretisch könnte das Frame als gültig verarbeitet worden sein.
         * Da aber das aktuelle Byte schon "angefangen" wurde zu verarbeiten, fehlt das in der Verarbeitungskette
         * und somit sind die nachfolgenden Bytes nicht verwertbar.
         */
        _rxState = RX_INVALID;
    }

    /*
     * Wenn der Markermodus in aktiv ist, soll das Byte normal verarbeitet werden.
     * Wenn der Markermodus aktiv ist, dann darf ein U_FRAME_END_IND Byte nur verarbeitet werden, wenn des vorherige Byte auch ein U_FRAME_END_IND war.
     */
    else if (!markerMode() || byte != U_FRAME_END_IND || (byte == U_FRAME_END_IND && _rxMarker))
    {
        // Setze den Marker wieder zurück falls aktiv
        _rxMarker = false;
        // Übernehme das Byte
        _rxFrame->addByte(byte);

        // Wenn der Busmonitor gestartet wurde, findet keine Verarbeitung - also auch kein ACKing
        if (!_monitoring)
        {
            // Wenn mehr als 7 bytes vorhanden kann geschaut werden ob das Frame für "mich" bestimmt ist
            if (_rxFrame->size() == 7)
            {
                // Prüfe ob ich für das Frame zuständig bin
                TPAckType ack = _cb.isAckRequired(_rxFrame->destination(), _rxFrame->isGroupAddress());
                if (_forceAck || ack)
                {
                    /*
                     * Speichere die Zuständigkeit dass dieses Frame weiterverarbeitet werden soll.
                     * Da es keine extra Funktion außer dem isAckRequired gibt, wird das erstmal gleich behandelt.
                     * Eine spätere Unterscheidung (ggf für Routermodus) muss man dann schauen.
                     */

                    _rxFrame->addFlags(TP_FRAME_FLAG_ADDRESSED);

                    // Das darf man natürlich nur wenn ich nicht gerade selber sende, da man eigene Frames nicht ACKt
                    if (_txState == TX_IDLE)
                    {
                        // Speichere das ein Acking erfolgen soll
                        _rxFrame->addFlags(TP_FRAME_FLAG_ACK);

                        // und im TPUart damit dieser das ACK schicken kann
                        _platform.writeUart(U_ACK_REQ | ack);
                    }
                }
            }

#ifdef USE_TP_RX_QUEUE
            // Prüfe nun ob die RxQueue noch Platz hat für Frame + Size (2) + Flags(1)
            if (_rxFrame->size() == 8 && (_rxFrame->flags() & TP_FRAME_FLAG_ADDRESSED))
            {
                if (availableInRxQueue() < (_rxFrame->size() + 3))
                {
                    // Nur wenn ich nicht selber sende
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
     * Wenn kein Markermodus aktiv ist, dann muss anhand des Frame geschaut werden, ob es Vollständig ist.
     * isFull prüft hier ob die maxSize oder die Längenangabe des Frames überschritten wurde!
     * In beiden Fällen muss das Frame verarbeitet werden.
     */
    if (!markerMode() && (_rxFrame->isFull()))
    {
        processRxFrameComplete();
    }
}

/*
 * Verarbeitet das aktuelle Frame und prüft ob dieses vollständig und gültig (checksum) ist.
 * Sollte ein Frame vollständig und gültig sein, so wird deses falls für "mich" bestimmt in die Queue gelegt und der Modus ist wieder RX_IDLE.
 * Ansonsten wird das Frame als ungültig verworfen und der Zustand ist RX_INVALID, da nicht sichergestellt ist das Folgebytes wieder Steuercodes sind.
 * Ausnahme im Markermodus, hier wird der Status RX_INVALID an einer anderen Stelle direkt wieder auf RX_IDLE geändert, weil
 * dann ist sicher gestellt, dass das das Frame auf TP Ebene kaputt gegangen ist.
 */
void TpUartDataLinkLayer::processRxFrameComplete()
{
    // Sollte aktuell kein Frame in der Bearbeitung sein, dann breche ab
    if (!_rxFrame->size())
        return;

    // Ist das Frame vollständig und gültig
    if (_rxFrame->isValid())
    {
        // Wenn ein Frame gesendet wurde
        if (_txState == TX_FRAME)
        {
            // prüfe ob das Empfangen diesem entspricht: Vergleich der Quelladresse und Zieladresse sowie Startbyte ohne Berücksichtigung des Retry-Bits
            if(!((_rxFrame->data(0) ^ _txFrame->data(0)) & ~0x20) && _rxFrame->destination() == _txFrame->destination() && _rxFrame->source() == _txFrame->source())
            {
                // und markiere das entsprechend
                // println("MATCH");
                _rxFrame->addFlags(TP_FRAME_FLAG_ECHO);
            }
            // Jetzt warte noch auf das L_DATA_CON
        }
        // wenn das frame für mich ist oder ich im busmonitor modus bin dann möchte ich es weiter verarbeiten
        if (_rxFrame->flags() & TP_FRAME_FLAG_ADDRESSED || _monitoring)
        {
            /*
             * Im Busmonitormodus muss noch auf ein Ack gewartet werden.
             * Daher wird hier der Status geändert und vor dem echten abschließen zurück gesprungen.
             * Sobald ein weiteres mal aufgerufen wird, (egal ob geackt oder nicht) wird das Frame geschlossen.
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
            // Sonst verwerfe das Paket und gebe den Speicher frei -> da nicht in die Queue gepackt wird
            _rxIgnoredFrameCounter++;
        }
        // Und wieder bereit für Steuercodes
        _rxState = RX_IDLE;
    }
    else
    {
        /*
         * Ist das Frame unvollständig oder ungültig dann wechsle in RX_INVALID Modus da ich nicht unterscheiden kann,
         * ob es sich um einen TPBus Error oder ein UART Error oder ein Timming Error handelt.
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

    // resete den aktuellen Framepointer
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
 * Steckt das zu sendende Frame in eine Queue, da der TpUart vielleicht gerade noch nicht sende bereit ist.
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
 * Es soll regelmäßig der Status abgefragt werden um ein Disconnect des TPUart und dessen Status zu erkennen.
 * Außerdem soll regelmäßig die aktuelle Config bzw der Modus übermittelt werden, so dass nach einem Disconnect,
 * der TPUart im richtigen Zustand ist.
 */
void TpUartDataLinkLayer::requestState(bool force /* = false */)
{
    if (!force)
    {
        if (!(_rxState == RX_IDLE || _rxState == RX_INVALID))
            return;

        // Nur 1x pro Sekunde
        if ((millis() - _lastStateRequest) < 1000)
            return;
    }

    // println("requestState");

    // Sende Konfiguration bzw. Modus
    if (_monitoring)
        _platform.writeUart(U_BUSMON_REQ);
    else
        requestConfig();

    // Frage status an - wenn monitoring inaktiv
    if (!_monitoring)
        _platform.writeUart(U_STATE_REQ);

    _lastStateRequest = millis();
}

/*
 * Sendet die aktuelle Config an den Chip
 */
void TpUartDataLinkLayer::requestConfig()
{
    // println("requestConfig");
#ifdef NCN5120
    if (markerMode())
        _platform.writeUart(U_CONFIGURE_REQ | U_CONFIGURE_MARKER_REQ);
#endif

    // Abweichende Config
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
 * Ein vereinfachter Lockmechanismus der nur auf dem gleichen Core funktionert.
 * Also perfekt für ISR
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
 * Wenn ein TxFrame gesendet wurde, wird eine Bestätigung für den Versand erwartet.
 * Kam es aber zu einem ungültigen Frame oder Busdisconnect, bleibt die Bestätigung aus und der STack hängt im TX_FRAME fest.
 * Daher muss nach einer kurzen Wartezeit das Warten beendet werden.
 */
void TpUartDataLinkLayer::clearOutdatedTxFrame()
{
    if (_txState == TX_FRAME && (millis() - _txLastTime) > 1000)
        processTxFrameComplete(false);
}

/*
 * Hier werden die ausgehenden Frames aus der Warteschlange genomnmen und versendet.
 * Das passiert immer nur einzelnd, da nach jedem Frame, gewartet werden muss bis das Frame wieder reingekommen ist und das L_DATA_CON rein kommt.
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
 * Prüfe ob ich zu lange keine Daten mehr erhalten habe und setzte den Status auf nicht verbunden.
 * Im normalen Modus wird jede Sekunde der Status angefragt. Daher kann hier eine kurze Zeit gewählt werden.
 * Im Monitoringmodus gibt es eigentlichen Frames, daher ist hier eine längere Zeit verwendet.
 * Dennoch kommt es bei größeren Datenmengen zu vermuteten Disconnect, daher wird auch noch die RxQueue beachtet.
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
     * Sollte ein overflow erkannt worden sein, so wechsle in RX_INVALID.
     * Das greift aber nur im loop und nicht im ISR. Aber bei Nutzung von ISR und DMA sollte dieser Fall nie passieren.
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
 * Hiermit kann die Stromversorgung des V20V (VCC2)
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
     * Jedes Frame muss mit einem U_L_DATA_START_REQ eingeleitet werden und jedes Folgebyte mit einem weiteren Positionsbyte (6bit).
     * Da das Positionsbyte aus dem U_L_DATA_START_REQ + Position besteht und wir sowieso mit 0 starten, ist eine weitere
     * Unterscheidung nicht nötig.
     *
     * Das letzte Byte (Checksumme) verwendet allerdings das U_L_DATA_END_REQ + Postion!
     * Außerdem gibt es eine weitere Besondertheit bei Extendedframes bis 263 Bytes lang sein können reichen die 6bit nicht mehr.
     * Hier muss ein U_L_DATA_OFFSET_REQ + Position (3bit) vorangestellt werden. Somit stehten 9bits für die Postion bereit.
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

        if (i == (_txFrame->size() - 1)) // Letztes Bytes (Checksumme)
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
 * Liefert die Anzahl der Frames, die nicht verarbeitet werden konnte.
 */
uint32_t TpUartDataLinkLayer::getRxInvalidFrameCounter()
{
    return _rxInvalidFrameCounter;
}

/*
 * Liefert die Anzahl der Frames, welche gültig und für das Geräte bestimmt sind
 */
uint32_t TpUartDataLinkLayer::getRxProcessdFrameCounter()
{
    return _rxProcessdFrameCounter;
}

/*
 * Liefert die Anzahl der Frames, welche gültig aber nicht f+r das Gerät bestimmt sind
 */
uint32_t TpUartDataLinkLayer::getRxIgnoredFrameCounter()
{
    return _rxIgnoredFrameCounter;
}

/*
 * Liefert die Anzahl der gezählten Steuerbytes, welche nicht erkannt wurden
 */
uint32_t TpUartDataLinkLayer::getRxUnknownControlCounter()
{
    return _rxUnkownControlCounter;
}

/*
 * Liefert die Anzahl der zusendenden Frames
 */
uint32_t TpUartDataLinkLayer::getTxFrameCounter()
{
    return _txFrameCounter;
}
/*
 * Liefert die Anzahl der versendeten Frames
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
 * Bei einem RP2040 wo beim "erase" eines Blocks auch der ISR gesperrt sind,
 * kann zwischern den Erases die Verarbeitung nachgeholt werden. So wird das Risiko von Frameverlusten deutlich minimiert.
 */
void __isr __time_critical_func(TpUartDataLinkLayer::processRxISR)()
{
    processRx(true);
}

/*
 * Steckt das empfangene Frame in eine Queue. Diese Queue ist notwendig,
 * weil ein Frame optional über ein ISR empfangen werden kann und die verarbeitung noch normal im knx.loop statt finden muss.
 * Außerdem ist diese Queue statisch vorallokierte, da in einem ISR keine malloc u.ä. gemacht werden kann.
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
    _rxBufferFront = (_rxBufferFront + 1) % MAX_RX_QUEUE_BYTES;
}

uint8_t TpUartDataLinkLayer::pullByteFromRxQueue()
{
    uint8_t byte = _rxBuffer[_rxBufferRear];
    _rxBufferRear = (_rxBufferRear + 1) % MAX_RX_QUEUE_BYTES;
    return byte;
}

uint16_t TpUartDataLinkLayer::availableInRxQueue()
{
    return ((_rxBufferFront == _rxBufferRear) ? MAX_RX_QUEUE_BYTES : (((MAX_RX_QUEUE_BYTES - _rxBufferFront) + _rxBufferRear) % MAX_RX_QUEUE_BYTES)) - 1;
}
#endif

#endif