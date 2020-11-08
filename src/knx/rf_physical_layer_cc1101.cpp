#ifndef DeviceFamily_CC13X0

#include "config.h"
#ifdef USE_RF

#include "rf_physical_layer_cc1101.h"
#include "rf_data_link_layer.h"

#include "bits.h"
#include "platform.h"

#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))
#define ABS(x)    ((x > 0) ? (x) : (-x))

// Table for encoding 4-bit data into a 8-bit Manchester encoding.
const uint8_t RfPhysicalLayerCC1101::manchEncodeTab[16] =   {0xAA,  // 0x0 Manchester encoded
                                                       0xA9,  // 0x1 Manchester encoded
                                                       0xA6,  // 0x2 Manchester encoded
                                                       0xA5,  // 0x3 Manchester encoded
                                                       0x9A,  // 0x4 Manchester encoded
                                                       0x99,  // 0x5 Manchester encoded
                                                       0x96,  // 0x6 Manchester encoded
                                                       0x95,  // 0x7 Manchester encoded
                                                       0x6A,  // 0x8 Manchester encoded
                                                       0x69,  // 0x9 Manchester encoded
                                                       0x66,  // 0xA Manchester encoded
                                                       0x65,  // 0xB Manchester encoded
                                                       0x5A,  // 0xC Manchester encoded
                                                       0x59,  // 0xD Manchester encoded
                                                       0x56,  // 0xE Manchester encoded
                                                       0x55}; // 0xF Manchester encoded

// Table for decoding 4-bit Manchester encoded data into 2-bit
// data. 0xFF indicates invalid Manchester encoding
const uint8_t RfPhysicalLayerCC1101::manchDecodeTab[16] = {0xFF, //  Manchester encoded 0x0 decoded
                                                     0xFF, //  Manchester encoded 0x1 decoded
                                                     0xFF, //  Manchester encoded 0x2 decoded
                                                     0xFF, //  Manchester encoded 0x3 decoded
                                                     0xFF, //  Manchester encoded 0x4 decoded
                                                     0x03, //  Manchester encoded 0x5 decoded
                                                     0x02, //  Manchester encoded 0x6 decoded
                                                     0xFF, //  Manchester encoded 0x7 decoded
                                                     0xFF, //  Manchester encoded 0x8 decoded
                                                     0x01, //  Manchester encoded 0x9 decoded
                                                     0x00, //  Manchester encoded 0xA decoded
                                                     0xFF, //  Manchester encoded 0xB decoded
                                                     0xFF, //  Manchester encoded 0xC decoded
                                                     0xFF, //  Manchester encoded 0xD decoded
                                                     0xFF, //  Manchester encoded 0xE decoded
                                                     0xFF};//  Manchester encoded 0xF decoded

// Product = CC1101
// Chip version = A   (VERSION = 0x04)
// Crystal accuracy = 10 ppm
// X-tal frequency = 26 MHz
// RF output power = + 10 dBm
// RX filterbandwidth = 270 kHz
// Deviation = 47 kHz
// Datarate = 32.73 kBaud
// Modulation = (0) 2-FSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 868.299866 MHz
// Channel spacing = 199.951172 kHz
// Channel number = 0
// Optimization = -
// Sync mode = (5) 15/16 + carrier-sense above threshold
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (0) CRC disabled for TX and RX
// Forward Error Correction = (0) FEC disabled
// Length configuration = (0) Fixed length packets, length configured in PKTLEN register.
// Packetlength = 255
// Preamble count = (2)  4 bytes
// Append status = 0
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = ( 0) Asserts when RX FiFO threshold
const uint8_t RfPhysicalLayerCC1101::cc1101_2FSK_32_7_kb[CFG_REGISTER] = {
                                        0x00,  // IOCFG2        GDO2 Output Pin Configuration
                                        0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                                        0x06,  // IOCFG0        GDO0 Output Pin Configuration
                                        0x40,  // FIFOTHR       RX FIFO and TX FIFO Thresholds // 4 bytes in RX FIFO (2 bytes manchester encoded)
                                        0x76,  // SYNC1         Sync Word
                                        0x96,  // SYNC0         Sync Word
                                        0xFF,  // PKTLEN        Packet Length
                                        0x00,  // PKTCTRL1      Packet Automation Control         
                                        0x00,  // PKTCTRL0      Packet Automation Control
                                        0x00,  // ADDR          Device Address
                                        0x00,  // CHANNR        Channel Number
                                        0x08,  // FSCTRL1       Frequency Synthesizer Control
                                        0x00,  // FSCTRL0       Frequency Synthesizer Control
                                        0x21,  // FREQ2         Frequency Control Word
                                        0x65,  // FREQ1         Frequency Control Word
                                        0x6A,  // FREQ0         Frequency Control Word
                                        0x6A,  // MDMCFG4       Modem Configuration
                                        0x4A,  // MDMCFG3       Modem Configuration
                                        0x05,  // MDMCFG2       Modem Configuration
                                        0x22,  // MDMCFG1       Modem Configuration
                                        0xF8,  // MDMCFG0       Modem Configuration
                                        0x47,  // DEVIATN       Modem Deviation Setting
                                        0x07,  // MCSM2         Main Radio Control State Machine Configuration
                                        0x30,  // MCSM1         Main Radio Control State Machine Configuration (IDLE after TX and RX)
                                        0x18,  // MCSM0         Main Radio Control State Machine Configuration
                                        0x2E,  // FOCCFG        Frequency Offset Compensation Configuration
                                        0x6D,  // BSCFG         Bit Synchronization Configuration
                                        0x43,  // AGCCTRL2      AGC Control       0x04,   // AGCCTRL2   magn target 33dB vs 36dB (max LNA+LNA2 gain vs. ) (highest gain cannot be used vs. all gain settings)
                                        0x40,  // AGCCTRL1      AGC Control       0x09,   // AGCCTRL1   carrier sense threshold disabled vs. 7dB below magn target (LNA prio strat. 1 vs 0)
                                        0x91,  // AGCCTRL0      AGC Control       0xB2,   // AGCCTRL0   channel filter samples 16 vs.32
                                        0x87,  // WOREVT1       High Byte Event0 Timeout
                                        0x6B,  // WOREVT0       Low Byte Event0 Timeout
                                        0xFB,  // WORCTRL       Wake On Radio Control
                                        0xB6,  // FREND1        Front End RX Configuration
                                        0x10,  // FREND0        Front End TX Configuration
                                        0xE9,  // FSCAL3        Frequency Synthesizer Calibration     0xEA,   // FSCAL3
                                        0x2A,  // FSCAL2        Frequency Synthesizer Calibration
                                        0x00,  // FSCAL1        Frequency Synthesizer Calibration
                                        0x1F,  // FSCAL0        Frequency Synthesizer Calibration
                                        0x41,  // RCCTRL1       RC Oscillator Configuration
                                        0x00,  // RCCTRL0       RC Oscillator Configuration
                                        0x59,  // FSTEST        Frequency Synthesizer Calibration Control
                                        0x7F,  // PTEST         Production Test
                                        0x3F,  // AGCTEST       AGC Test
                                        0x81,  // TEST2         Various Test Settings
                                        0x35,  // TEST1         Various Test Settings
                                        0x09   // TEST0         Various Test Settings
};

                                    //Patable index: -30  -20- -15  -10   0    5    7    10 dBm
const uint8_t RfPhysicalLayerCC1101::paTablePower868[8] = {0x03,0x17,0x1D,0x26,0x50,0x86,0xCD,0xC0};

RfPhysicalLayerCC1101::RfPhysicalLayerCC1101(RfDataLinkLayer& rfDataLinkLayer, Platform& platform)
    : RfPhysicalLayer(rfDataLinkLayer, platform)
{
}

void RfPhysicalLayerCC1101::manchEncode(uint8_t *uncodedData, uint8_t *encodedData)
{
  uint8_t  data0, data1;

  // - Shift to get 4-bit data values
  data1 = (((*uncodedData) >> 4) & 0x0F);
  data0 = ((*uncodedData) & 0x0F);

  // - Perform Manchester encoding -
  *encodedData       = (manchEncodeTab[data1]);
  *(encodedData + 1) = manchEncodeTab[data0];
}

bool RfPhysicalLayerCC1101::manchDecode(uint8_t *encodedData, uint8_t *decodedData)
{
  uint8_t data0, data1, data2, data3;

  // - Shift to get 4 bit data and decode
  data3 = ((*encodedData >> 4) & 0x0F);
  data2 = ( *encodedData       & 0x0F);
  data1 = ((*(encodedData + 1) >> 4) & 0x0F);
  data0 = ((*(encodedData + 1))      & 0x0F);

  // Check for invalid Manchester encoding
  if ( (manchDecodeTab[data3] == 0xFF ) | (manchDecodeTab[data2] == 0xFF ) |
     (manchDecodeTab[data1] == 0xFF ) | (manchDecodeTab[data0] == 0xFF ) )
  {
    return false;
  }

  // Shift result into a byte
  *decodedData = (manchDecodeTab[data3] << 6) | (manchDecodeTab[data2] << 4) |
                 (manchDecodeTab[data1] << 2) |  manchDecodeTab[data0];

  return true;
}

uint8_t RfPhysicalLayerCC1101::sIdle()
{
    uint8_t marcState;
    uint32_t timeStart;

    spiWriteStrobe(SIDLE);              //sets to idle first. must be in

    marcState = 0xFF;                   //set unknown/dummy state value
    timeStart = millis();
    while((marcState != MARCSTATE_IDLE) && ((millis() - timeStart) < CC1101_TIMEOUT))   //0x01 = sidle
    {
        marcState = (spiReadRegister(MARCSTATE) & MARCSTATE_BITMASK); //read out state of cc1101 to be sure in RX
    }

    //print("marcstate: 0x");
    //println(marcState, HEX);

    if(marcState != MARCSTATE_IDLE)
    {
        println("Timeout when trying to set idle state.");
        return false;
    }

    return true;
}

uint8_t RfPhysicalLayerCC1101::sReceive()
{
    uint8_t marcState;
    uint32_t timeStart;

    spiWriteStrobe(SRX);                //writes receive strobe (receive mode)

    marcState = 0xFF;                   //set unknown/dummy state value
    timeStart = millis();
    while((marcState != MARCSTATE_RX) && ((millis() - timeStart) < CC1101_TIMEOUT))              //0x0D = RX
    {
        marcState = (spiReadRegister(MARCSTATE) & MARCSTATE_BITMASK); //read out state of cc1101 to be sure in RX
    }

    //print("marcstate: 0x");
    //println(marcState, HEX);

    if(marcState != MARCSTATE_RX)
    {
        println("Timeout when trying to set receive state.");
        return false;
    }

    return true;
}

void RfPhysicalLayerCC1101::spiWriteRegister(uint8_t spi_instr, uint8_t value)
{
     uint8_t tbuf[2] = {0};
     tbuf[0] = spi_instr | WRITE_SINGLE_BYTE;
     tbuf[1] = value;
     uint8_t len = 2;
     digitalWrite(SPI_SS_PIN, LOW);
     _platform.readWriteSpi(tbuf, len);
     digitalWrite(SPI_SS_PIN, HIGH);
}

uint8_t RfPhysicalLayerCC1101::spiReadRegister(uint8_t spi_instr)
{
     uint8_t value;
     uint8_t rbuf[2] = {0};
     rbuf[0] = spi_instr | READ_SINGLE_BYTE;
     uint8_t len = 2;
     digitalWrite(SPI_SS_PIN, LOW);
     _platform.readWriteSpi(rbuf, len);
     digitalWrite(SPI_SS_PIN, HIGH);
     value = rbuf[1];
     //printf("SPI_arr_0: 0x%02X\n", rbuf[0]);
     //printf("SPI_arr_1: 0x%02X\n", rbuf[1]);
     return value;
}

uint8_t RfPhysicalLayerCC1101::spiWriteStrobe(uint8_t spi_instr)
{
     uint8_t tbuf[1] = {0};
     tbuf[0] = spi_instr;
     //printf("SPI_data: 0x%02X\n", tbuf[0]);
     digitalWrite(SPI_SS_PIN, LOW);
     _platform.readWriteSpi(tbuf, 1);
     digitalWrite(SPI_SS_PIN, HIGH);
     return tbuf[0];
}

void RfPhysicalLayerCC1101::spiReadBurst(uint8_t spi_instr, uint8_t *pArr, uint8_t len)
{
     uint8_t rbuf[len + 1];
     rbuf[0] = spi_instr | READ_BURST;
     digitalWrite(SPI_SS_PIN, LOW);
     _platform.readWriteSpi(rbuf, len + 1);
     digitalWrite(SPI_SS_PIN, HIGH);
     for (uint8_t i=0; i<len ;i++ )
     {
          pArr[i] = rbuf[i+1];
          //printf("SPI_arr_read: 0x%02X\n", pArr[i]);
     }
}

void RfPhysicalLayerCC1101::spiWriteBurst(uint8_t spi_instr, const uint8_t *pArr, uint8_t len)
{
     uint8_t tbuf[len + 1];
     tbuf[0] = spi_instr | WRITE_BURST;
     for (uint8_t i=0; i<len ;i++ )
     {
          tbuf[i+1] = pArr[i];
          //printf("SPI_arr_write: 0x%02X\n", tbuf[i+1]);
     }
     digitalWrite(SPI_SS_PIN, LOW);
     _platform.readWriteSpi(tbuf, len + 1);
     digitalWrite(SPI_SS_PIN, HIGH);
}

void RfPhysicalLayerCC1101::powerDownCC1101()
{
    // Set IDLE state first
    sIdle();
    delayMicroseconds(100);
    // CC1101 Power Down
    spiWriteStrobe(SPWD);               
}

void RfPhysicalLayerCC1101::setOutputPowerLevel(int8_t dBm)
{
    uint8_t pa = 0xC0;

    if      (dBm <= -30) pa = 0x00;
    else if (dBm <= -20) pa = 0x01;
    else if (dBm <= -15) pa = 0x02;
    else if (dBm <= -10) pa = 0x03;
    else if (dBm <= 0)   pa = 0x04;
    else if (dBm <= 5)   pa = 0x05;
    else if (dBm <= 7)   pa = 0x06;
    else if (dBm <= 10)  pa = 0x07;

    spiWriteRegister(FREND0, pa);
}

bool RfPhysicalLayerCC1101::InitChip()
{
    // Setup SPI and GPIOs
    _platform.setupSpi();
    pinMode(GPIO_GDO2_PIN, INPUT);
    pinMode(GPIO_GDO0_PIN, INPUT);
    pinMode(SPI_SS_PIN, OUTPUT);

    // Toggle chip select signal as described in CC11xx manual
    digitalWrite(SPI_SS_PIN, HIGH); 
    delayMicroseconds(30);
    digitalWrite(SPI_SS_PIN, LOW);
    delayMicroseconds(30);
    digitalWrite(SPI_SS_PIN, HIGH); 
    delayMicroseconds(45);
   
    // Send SRES command   
    digitalWrite(SPI_SS_PIN, LOW);
    delay(10); // Normally we would have to poll MISO here: while(_platform.readGpio(SPI_MISO_PIN));
    spiWriteStrobe(SRES);
    // Wait for chip to finish internal reset   
    delay(10); // Normally we would have to poll MISO here: while(_platform.readGpio(SPI_MISO_PIN));
    digitalWrite(SPI_SS_PIN, HIGH); 

    // Flush the FIFOs
    spiWriteStrobe(SFTX);
    delayMicroseconds(100);
    spiWriteStrobe(SFRX);
    delayMicroseconds(100);

    uint8_t partnum = spiReadRegister(PARTNUM); //reads CC1101 partnumber;
    uint8_t version = spiReadRegister(VERSION); //reads CC1101 version number;

    // Checks if valid chip ID is found. Usually 0x03 or 0x14. if not -> abort
    if(version == 0x00 || version == 0xFF)
    {
        println("No CC11xx found!");
        stopChip();
        return false;
    }

    print("Partnumber: 0x"); 
    println(partnum, HEX);
    print("Version   : 0x");
    println(version, HEX);

    // Set modulation mode 2FSK, 32768kbit/s
    spiWriteBurst(WRITE_BURST,cc1101_2FSK_32_7_kb,CFG_REGISTER);

    // Set PA table 
    spiWriteBurst(PATABLE_BURST, paTablePower868, 8);

    // Set ISM band to 868.3MHz
    spiWriteRegister(FREQ2,0x21);
    spiWriteRegister(FREQ1,0x65);
    spiWriteRegister(FREQ0,0x6A);

    // Set channel 0 in ISM band
    spiWriteRegister(CHANNR, 0);

    // Set PA to 0dBm as default
    setOutputPowerLevel(0);

    return true;
}

void RfPhysicalLayerCC1101::stopChip()
{
    powerDownCC1101();

    _platform.closeSpi();
}

void RfPhysicalLayerCC1101::showRegisterSettings()
{
    uint8_t config_reg_verify[CFG_REGISTER];
    uint8_t Patable_verify[CFG_REGISTER];

    spiReadBurst(READ_BURST,config_reg_verify,CFG_REGISTER);  //reads all 47 config register from cc1101
    spiReadBurst(PATABLE_BURST,Patable_verify,8);             //reads output power settings from cc1101

    println("Config Register:");
    printHex("", config_reg_verify, CFG_REGISTER);

    println("PaTable:");
    printHex("", Patable_verify, 8);
}

void RfPhysicalLayerCC1101::loop()
{
    switch (_loopState)
    {
        case TX_START:
        {
            prevStatusGDO0 = 0;
            prevStatusGDO2 = 0;
            // Set sync word in TX mode
            // The same sync word is used in RX mode, but we use it in different way here:
            // Important: the TX FIFO must provide the last byte of the 
            // sync word
            spiWriteRegister(SYNC1, 0x54);
            spiWriteRegister(SYNC0, 0x76);
            // Set TX FIFO threshold to 33 bytes
            spiWriteRegister(FIFOTHR, 0x47);
            // Set GDO2 to be TX FIFO threshold signal
            spiWriteRegister(IOCFG2, 0x02);        
            // Set GDO0 to be packet transmitted signal
            spiWriteRegister(IOCFG0, 0x06);            
            // Flush TX FIFO
            spiWriteStrobe(SFTX);

            _rfDataLinkLayer.loadNextTxFrame(&sendBuffer, &sendBufferLength);

            // Calculate total number of bytes in the KNX RF packet from L-field
            pktLen = PACKET_SIZE(sendBuffer[0]);
            // Check for valid length
            if ((pktLen == 0) || (pktLen > 290))
            {
                println("TX packet length error!");
                break;
            }

            // Manchester encoded data takes twice the space plus            
            // 1 byte for postamble and 1 byte (LSB) of the synchronization word
            bytesLeft = (2 * pktLen) + 2;
            // Last byte of synchronization word  
            buffer[0] = 0x96;
            // Manchester encode packet 
            for (int i = 0; i < pktLen; i++)
            {
                manchEncode(&sendBuffer[i], &buffer[1 + i*2]);
            }
            // Append the postamble sequence
            buffer[1 + bytesLeft - 1] = 0x55;

            // Fill TX FIFO
            pByteIndex = &buffer[0];
            // Set fixed packet length mode if less than 256 bytes to transmit
            if (bytesLeft < 256)
            {
                spiWriteRegister(PKTLEN, bytesLeft);
                spiWriteRegister(PKTCTRL0, 0x00); // Set fixed pktlen mode
                fixedLengthMode = true;
            } 
            else // Else set infinite length mode
            {
                uint8_t fixedLength = bytesLeft % 256;
                spiWriteRegister(PKTLEN, fixedLength);    
                spiWriteRegister(PKTCTRL0, 0x02); 
                fixedLengthMode = false;
            }

            uint8_t bytesToWrite = MIN(64, bytesLeft);  
            spiWriteBurst(TXFIFO_BURST, pByteIndex, bytesToWrite);
            pByteIndex += bytesToWrite;
            bytesLeft  -= bytesToWrite;

            // Enable transmission of packet
            spiWriteStrobe(STX);

            _loopState = TX_ACTIVE;
        }
        // Fall through
        
        case TX_ACTIVE:
        {
            // Check if we have an incomplete packet transmission
            if (syncStart && (millis() - packetStartTime > TX_PACKET_TIMEOUT))
            {
                println("TX packet timeout!");
                // Set transceiver to IDLE (no RX or TX)
                sIdle();
                _loopState = TX_END;
                break;
            }

            // Detect falling edge 1->0 on GDO2
            statusGDO2 = digitalRead(GPIO_GDO2_PIN);
            if(prevStatusGDO2 != statusGDO2)
            {
                prevStatusGDO2 = statusGDO2;

                // Check if signal GDO2 is de-asserted (TX FIFO is below threshold of 33 bytes, i.e. TX FIFO is half full)
                if(statusGDO2 == 0)                      
                {
                    // - TX FIFO half full detected (< 33 bytes)
                    // Write data fragment to TX FIFO
                    uint8_t bytesToWrite = MIN(64, bytesLeft);
                    spiWriteBurst(TXFIFO_BURST, pByteIndex, bytesToWrite);
                    pByteIndex += bytesToWrite;
                    bytesLeft  -= bytesToWrite;

                    // Set fixed length mode if less than 256 left to transmit
                    if ( (bytesLeft < (256 - 64)) && !fixedLengthMode )
                    {
                        spiWriteRegister(PKTCTRL0, 0x00); // Set fixed pktlen mode
                        fixedLengthMode = true;
                    }
                }
            }

            // Detect falling edge 1->0 on GDO0                
            statusGDO0 = digitalRead(GPIO_GDO0_PIN);
            if(prevStatusGDO0 != statusGDO0)
            {
                prevStatusGDO0 = statusGDO0;

                // If GDO0 is de-asserted: TX packet complete or TX FIFO underflow
                if (statusGDO0 == 0x00)
                {
                    // There might be an TX FIFO underflow
                    uint8_t chipStatusBytes = spiWriteStrobe(SNOP);
                    if ((chipStatusBytes & CHIPSTATUS_STATE_BITMASK) == CHIPSTATUS_STATE_TX_UNDERFLOW)
                    {
                        println("TX FIFO underflow!");
                       // Set transceiver to IDLE (no RX or TX)
                       sIdle();
                    }
                    _loopState = TX_END;
                }
                else
                {
                    // GDO0 asserted because sync word was transmitted
                    //println("TX Syncword!");
                    // wait for TX_PACKET_TIMEOUT milliseconds
                    // Complete packet must have been transmitted within this time
                    packetStartTime = millis();
                    syncStart = true;
                }
            }
        }
        break;

        case TX_END:
        {
            // free buffer
            delete sendBuffer;
            // Go back to RX after TX
            _loopState = RX_START;
        }
        break;

        case RX_START:
        {
            prevStatusGDO2 = 0;
            prevStatusGDO0 = 0;
            syncStart = false;
            packetStart = true;
            fixedLengthMode = false;
            pByteIndex = buffer;
            bytesLeft = 0;
            pktLen = 0;
            // Set sync word in RX mode
            // The same sync word is used in TX mode, but we use it in different way
            spiWriteRegister(SYNC1, 0x76);
            spiWriteRegister(SYNC0, 0x96);
            // Set GDO2 to be RX FIFO threshold signal
            spiWriteRegister(IOCFG2, 0x00);              
            // Set GDO0 to be packet received signal
            spiWriteRegister(IOCFG0, 0x06);            
            // Set RX FIFO threshold to 4 bytes
            spiWriteRegister(FIFOTHR, 0x40);
            // Set infinite pktlen mode
            spiWriteRegister(PKTCTRL0, 0x02); 
            // Flush RX FIFO
            spiWriteStrobe(SFRX);
            // Start RX
            sReceive();
            _loopState = RX_ACTIVE;
        }
        break;

        case RX_ACTIVE:
        {
            if (!_rfDataLinkLayer.isTxQueueEmpty() && !syncStart)
            {
                sIdle();
                _loopState = TX_START;
                break;
            }

            // Check if we have an incomplete packet reception
            // This is related to CC1101 errata "Radio stays in RX state instead of entering RXFIFO_OVERFLOW state"
            if (syncStart && (millis() - packetStartTime > RX_PACKET_TIMEOUT))
            {
                println("RX packet timeout!");
                //uint8_t marcState = (spiReadRegister(MARCSTATE) & MARCSTATE_BITMASK); //read out state of cc1101 to be sure in RX
                //print("marcstate: 0x");
                //println(marcState, HEX);
                sIdle();
                _loopState = RX_START;
                break;
            }

            // Detect rising edge 0->1 on GDO2
            statusGDO2 = digitalRead(GPIO_GDO2_PIN);
            if(prevStatusGDO2 != statusGDO2)
            {
                prevStatusGDO2 = statusGDO2;

                // Check if signal GDO2 is asserted (RX FIFO is equal to or above threshold of 4 bytes)
                if(statusGDO2 == 1)                      
                {
                    if (packetStart)
                    {
                        // - RX FIFO 4 bytes detected - 
                        // Calculate the total length of the packet, and set fixed mode if less
                        // than 255 bytes to receive

                        // Read the 2 first bytes
                        spiReadBurst(RXFIFO_BURST, pByteIndex, 2);   

                        // Decode the L-field
                        if (!manchDecode(&buffer[0], &packet[0]))
                        {
                            //println("Could not decode L-field: manchester code violation");
                            _loopState = RX_START;           
                            break;
                        }
                        // Get bytes to receive from L-field, multiply by 2 because of manchester code
                        pktLen = 2 * PACKET_SIZE(packet[0]);

                        // - Length mode - 
                        if (pktLen < 256)
                        {
                            // Set fixed packet length mode is less than 256 bytes
                            spiWriteRegister(PKTLEN, pktLen);
                            spiWriteRegister(PKTCTRL0, 0x00); // Set fixed pktlen mode
                            fixedLengthMode = true;
                        }
                        else
                        {
                            // Infinite packet length mode is more than 255 bytes
                            // Calculate the PKTLEN value
                            uint8_t fixedLength = pktLen  % 256;
                            spiWriteRegister(PKTLEN, fixedLength);    
                        } 
                                    
                        pByteIndex += 2;
                        bytesLeft = pktLen - 2;
                        
                        // Set RX FIFO threshold to 32 bytes
                        packetStart = false;
                        spiWriteRegister(FIFOTHR, 0x47);
                    }
                    else
                    {
                        // - RX FIFO Half Full detected - 
                        // Read out the RX FIFO and set fixed mode if less
                        // than 255 bytes to receive

                        // - Length mode -
                        // Set fixed packet length mode if less than 256 bytes
                        if ((bytesLeft < 256 ) && !fixedLengthMode)
                        {
                            spiWriteRegister(PKTCTRL0, 0x00); // Set fixed pktlen mode  
                            fixedLengthMode = true;
                        }

                        // Read out the RX FIFO
                        // Do not empty the FIFO (See the CC110x or 2500 Errata Note)
                        spiReadBurst(RXFIFO_BURST, pByteIndex, 32 - 1);   

                        bytesLeft  -= (32 - 1);
                        pByteIndex += (32 - 1);
                    }

                }
            }

            // Detect falling edge 1->0 on GDO0                
            statusGDO0 = digitalRead(GPIO_GDO0_PIN);
            if(prevStatusGDO0 != statusGDO0)
            {
                prevStatusGDO0 = statusGDO0;

                // If GDO0 is de-asserted: RX packet complete or RX FIFO overflow
                if (statusGDO0 == 0x00)
                {
                    // There might be an RX FIFO overflow
                    uint8_t chipStatusBytes = spiWriteStrobe(SNOP);
                    if ((chipStatusBytes & CHIPSTATUS_STATE_BITMASK) == CHIPSTATUS_STATE_RX_OVERFLOW)
                    {
                        println("RX FIFO overflow!");
                        _loopState = RX_START;           
                        break;
                    }

                    // Check if we are in the middle of the packet reception
                    if (!packetStart)
                    {
                        // Complete packet received
                        // Read out remaining bytes in the RX FIFO
                        spiReadBurst(RXFIFO_BURST, pByteIndex, bytesLeft); 
                        _loopState = RX_END;
                    }
                }
                else
                {
                    // GDO0 asserted because sync word was received and recognized
                    //println("RX Syncword!");
                    // wait for RX_PACKET_TIMEOUT milliseconds
                    // Complete packet must have been received within this time
                    packetStartTime = millis();
                    syncStart = true;
                }
                
            }
        }
        break;

        case RX_END:
        {
            uint16_t pLen = PACKET_SIZE(packet[0]);
            // Decode the first block (always 10 bytes + 2 bytes CRC)
            bool decodeOk = true;
            for (uint16_t i = 1; i < pLen; i++)
            {
                // Check for manchester violation, abort if there is one
                if(!manchDecode(&buffer[i*2], &packet[i]))
                {
                    println("Could not decode packet: manchester code violation");
                    decodeOk = false;
                    break;
                }
            }

            if (decodeOk)
            {
                _rfDataLinkLayer.frameBytesReceived(&packet[0], pLen);
            }

            _loopState = RX_START;
        }
        break;
    }
}

#endif // USE_RF

#endif // DeviceFamily_CC13X0
