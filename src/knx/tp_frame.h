#pragma once
#pragma GCC optimize("O3")

#include "cemi_frame.h"
#include <cstring>
#include <stdint.h>
#include <string>

// Means that the frame is invalid
#define TP_FRAME_FLAG_INVALID 0b10000000

// Means that the frame is an extended frame
#define TP_FRAME_FLAG_EXTENDED 0b01000000

// Means that the frame has been repeated
#define TP_FRAME_FLAG_REPEATED 0b00100000

// Means that the frame comes from the device itself
#define TP_FRAME_FLAG_ECHO 0b00010000

// Means that the frame is processed by this device
#define TP_FRAME_FLAG_ADDRESSED 0b00001000

// Means that the frame has been acked with BUSY
#define TP_FRAME_FLAG_ACK_BUSY 0b00000100

// Means that the frame has been acked with NACK
#define TP_FRAME_FLAG_ACK_NACK 0b00000010

// Means that the frame has been acked
#define TP_FRAME_FLAG_ACK 0b00000001

class TpFrame
{
  private:
    uint8_t *_data;
    uint16_t _size;
    uint16_t _maxSize;
    uint8_t _flags = 0;

    /*
     * Sets a few flags based on the control byte
     */
    inline void presetFlags()
    {
        if (isExtended())
            addFlags(TP_FRAME_FLAG_EXTENDED);

        if (isRepeated())
            addFlags(TP_FRAME_FLAG_REPEATED);
    }

  public:
    /*
     * Convert a CemiFrame into a TpFrame
     */
    TpFrame(CemiFrame &cemiFrame)
    {
        _size = cemiFrame.telegramLengthtTP();
        _maxSize = cemiFrame.telegramLengthtTP();
        _data = (uint8_t *)malloc(cemiFrame.telegramLengthtTP());
        cemiFrame.fillTelegramTP(_data);
        presetFlags();
    }

    /*
     * Create a TpFrame with a reserved space.
     * Used for incoming parsing.
     */
    TpFrame(uint16_t maxSize = 263)
        : _maxSize(maxSize)
    {
        _data = (uint8_t *)malloc(_maxSize);
        _size = 0;
    }

    /*
     * Free the data area
     */
    ~TpFrame()
    {
        free(_data);
    }

    /*
     * Add a byte at end.
     * Used for incoming parsing.
     */
    inline void addByte(uint8_t byte)
    {
        if (!isFull())
        {
            _data[_size] = byte;
            _size++;
        }

        // Read meta data for flags
        if (_size == 1)
            presetFlags();
    }

    /*
     * Current frame size. This may differ from the actual size as long as the frame is not complete.
     */
    inline uint16_t size()
    {
        return _size;
    }

    /*
     * Returns the assigned flags
     */
    inline uint16_t flags()
    {
        return _flags;
    }

    /*
     * Adds one or more flags
     */
    inline void addFlags(uint8_t flags)
    {
        _flags |= flags;
    }

    /*
     * Returns a pointer to the data
     */
    inline uint8_t *data()
    {
        return _data;
    }

    /*
     * Returns the byte corresponding to the specified position
     */
    inline uint8_t data(uint16_t pos)
    {
        return _data[pos];
    }

    /*
     * Resets the internal values to refill the frame.
     */
    inline void reset()
    {
        _size = 0;
        _flags = 0;
        // It is important to fill the _data with zeros so that the length is 0 as long as the value has not yet been read in.
        memset(_data, 0x0, _maxSize);
    }

    /*
     * Checks whether the frame has been imported completely
     */
    inline bool isFull()
    {
        return _size >= (_size >= 7 ? fullSize() : _maxSize);
    }

    /*
     * Returns is the frame exteneded or not
     */
    inline bool isExtended()
    {
        return (_data[0] & 0xD3) == 0x10;
    }

    /*
     * Returns the source
     * Assumes that enough data has been imported.
     */
    inline uint16_t source()
    {
        return isExtended() ? (_data[2] << 8) + _data[3] : (_data[1] << 8) + _data[2];
    }

    inline std::string humanSource()
    {
        uint16_t value = source();
        char buffer[10];
        sprintf(buffer, "%02i.%02i.%03i", (value >> 12 & 0b1111), (value >> 8 & 0b1111), (value & 0b11111111));
        return buffer;
    }

    inline std::string humanDestination()
    {
        uint16_t value = destination();
        char buffer[10];
        if (isGroupAddress())
            sprintf(buffer, "%02i/%02i/%03i", (value >> 11 & 0b1111), (value >> 8 & 0b111), (value & 0b11111111));
        else
            sprintf(buffer, "%02i.%02i.%03i", (value >> 12 & 0b1111), (value >> 8 & 0b1111), (value & 0b11111111));

        return buffer;
    }

    /*
     * Returns the destination
     * Assumes that enough data has been imported.
     */
    inline uint16_t destination()
    {
        return isExtended() ? (_data[4] << 8) + _data[5] : (_data[3] << 8) + _data[4];
    }

    /*
     * Returns the payload size (with checksum)
     * Assumes that enough data has been imported.
     */
    inline uint8_t payloadSize()
    {
        return isExtended() ? _data[6] : _data[5] & 0b1111;
    }

    /*
     * Returns the header size
     */
    inline uint8_t headerSize()
    {
        return isExtended() ? 9 : 8;
    }

    /*
     * Returns the frame size based on header and payload size.
     * Assumes that enough data has been imported.
     */
    inline uint16_t fullSize()
    {
        return headerSize() + payloadSize();
    }

    /*
     * Returns if the destination is a group address
     * Assumes that enough data has been imported.
     */
    inline bool isGroupAddress()
    {
        return isExtended() ? (_data[1] >> 7) & 0b1 : (_data[5] >> 7) & 0b1;
    }

    /*
     * Calculates the size of a CemiFrame. A CemiFrame has 2 additional bytes at the beginning.
     * An additional byte is added to a standard frame, as this still has to be converted into an extendend.
     */
    uint16_t cemiSize()
    {
        return fullSize() + (isExtended() ? 2 : 3) - 1; // -1 without CRC
    }

    /**
     * Creates a buffer and converts the TpFrame into a CemiFrame.
     * Important: After processing (i.e. also after using the CemiFrame), the reference must be released manually.
     */
    uint8_t *cemiData()
    {
        uint8_t *cemiBuffer = (uint8_t *)malloc(cemiSize());

        // Das CEMI erwartet die Daten im Extended format inkl. zwei zusätzlicher Bytes am Anfang.
        cemiBuffer[0] = 0x29;
        cemiBuffer[1] = 0x0;
        cemiBuffer[2] = _data[0];
        if (isExtended())
        {
            memcpy(cemiBuffer + 2, _data, fullSize() - 1); // -1 without CRC
        }
        else
        {
            cemiBuffer[3] = _data[5] & 0xF0;
            memcpy(cemiBuffer + 4, _data + 1, 4);
            cemiBuffer[8] = _data[5] & 0x0F;
            memcpy(cemiBuffer + 9, _data + 6, cemiBuffer[8] + 2 - 1); // -1 without CRC
        }

        return cemiBuffer;
    }

    /*
     * Checks whether the frame is complete and valid.
     */
    inline bool isValid()
    {
        if (!isComplete())
            return false;

        uint8_t sum = 0;
        const uint16_t s = fullSize() - 1;
        for (uint16_t i = 0; i < s; i++)
            sum ^= _data[i];
        return _data[s] == (uint8_t)~sum;
    }

    /*
     * Checks whether the frame is long enough to match the length specified in the frame
     */
    inline bool isComplete()
    {
        return _size == fullSize();
    }

    inline bool isRepeated()
    {
        return !(_data[0] & 0b100000);
    }
};