#pragma once

#ifndef DeviceFamily_CC13X0

#include "config.h"
#ifdef USE_RF

#include <stdint.h>

#include "rf_physical_layer.h"

/*----------------------------------[standard]--------------------------------*/
#define CC1101_TIMEOUT 		     2000		// Time to wait for a response from CC1101

#define RX_PACKET_TIMEOUT        20   // Wait 20ms for packet reception to complete
#define TX_PACKET_TIMEOUT        20   // Wait 20ms for packet reception to complete

#ifdef __linux__ // Linux Platform
extern void delayMicroseconds (unsigned int howLong);
#endif

/*----------------------[CC1101 - misc]---------------------------------------*/
#define CRYSTAL_FREQUENCY         26000000
#define CFG_REGISTER              0x2F  //47 registers
#define FIFOBUFFER                0x42  //size of Fifo Buffer +2 for rssi and lqi
#define RSSI_OFFSET_868MHZ        0x4E  //dec = 74
#define TX_RETRIES_MAX            0x05  //tx_retries_max
#define ACK_TIMEOUT               250   //ACK timeout in ms
#define CC1101_COMPARE_REGISTER   0x00  //register compare 0=no compare 1=compare
#define BROADCAST_ADDRESS         0x00  //broadcast address
#define CC1101_FREQ_315MHZ        0x01
#define CC1101_FREQ_434MHZ        0x02
#define CC1101_FREQ_868MHZ        0x03
#define CC1101_FREQ_915MHZ        0x04
#define CC1101_TEMP_ADC_MV        3.225 //3.3V/1023 . mV pro digit
#define CC1101_TEMP_CELS_CO       2.47  //Temperature coefficient 2.47mV per Grad Celsius

/*---------------------------[CC1101 - R/W offsets]---------------------------*/
#define WRITE_SINGLE_BYTE   0x00
#define WRITE_BURST         0x40
#define READ_SINGLE_BYTE    0x80
#define READ_BURST          0xC0
/*---------------------------[END R/W offsets]--------------------------------*/

/*------------------------[CC1101 - FIFO commands]----------------------------*/
#define TXFIFO_BURST        0x7F    //write burst only
#define TXFIFO_SINGLE_BYTE  0x3F    //write single only
#define RXFIFO_BURST        0xFF    //read burst only
#define RXFIFO_SINGLE_BYTE  0xBF    //read single only
#define PATABLE_BURST       0x7E    //power control read/write
#define PATABLE_SINGLE_BYTE 0xFE    //power control read/write
/*---------------------------[END FIFO commands]------------------------------*/

/*----------------------[CC1101 - config register]----------------------------*/
#define IOCFG2   0x00         // GDO2 output pin configuration
#define IOCFG1   0x01         // GDO1 output pin configuration
#define IOCFG0   0x02         // GDO0 output pin configuration
#define FIFOTHR  0x03         // RX FIFO and TX FIFO thresholds
#define SYNC1    0x04         // Sync word, high byte
#define SYNC0    0x05         // Sync word, low byte
#define PKTLEN   0x06         // Packet length
#define PKTCTRL1 0x07         // Packet automation control
#define PKTCTRL0 0x08         // Packet automation control
#define DEVADDR  0x09         // Device address
#define CHANNR   0x0A         // Channel number
#define FSCTRL1  0x0B         // Frequency synthesizer control
#define FSCTRL0  0x0C         // Frequency synthesizer control
#define FREQ2    0x0D         // Frequency control word, high byte
#define FREQ1    0x0E         // Frequency control word, middle byte
#define FREQ0    0x0F         // Frequency control word, low byte
#define MDMCFG4  0x10         // Modem configuration
#define MDMCFG3  0x11         // Modem configuration
#define MDMCFG2  0x12         // Modem configuration
#define MDMCFG1  0x13         // Modem configuration
#define MDMCFG0  0x14         // Modem configuration
#define DEVIATN  0x15         // Modem deviation setting
#define MCSM2    0x16         // Main Radio Cntrl State Machine config
#define MCSM1    0x17         // Main Radio Cntrl State Machine config
#define MCSM0    0x18         // Main Radio Cntrl State Machine config
#define FOCCFG   0x19         // Frequency Offset Compensation config
#define BSCFG    0x1A         // Bit Synchronization configuration
#define AGCCTRL2 0x1B         // AGC control
#define AGCCTRL1 0x1C         // AGC control
#define AGCCTRL0 0x1D         // AGC control
#define WOREVT1  0x1E         // High byte Event 0 timeout
#define WOREVT0  0x1F         // Low byte Event 0 timeout
#define WORCTRL  0x20         // Wake On Radio control
#define FREND1   0x21         // Front end RX configuration
#define FREND0   0x22         // Front end TX configuration
#define FSCAL3   0x23         // Frequency synthesizer calibration
#define FSCAL2   0x24         // Frequency synthesizer calibration
#define FSCAL1   0x25         // Frequency synthesizer calibration
#define FSCAL0   0x26         // Frequency synthesizer calibration
#define RCCTRL1  0x27         // RC oscillator configuration
#define RCCTRL0  0x28         // RC oscillator configuration
#define FSTEST   0x29         // Frequency synthesizer cal control
#define PTEST    0x2A         // Production test
#define AGCTEST  0x2B         // AGC test
#define TEST2    0x2C         // Various test settings
#define TEST1    0x2D         // Various test settings
#define TEST0    0x2E         // Various test settings
/*-------------------------[END config register]------------------------------*/

/*------------------------[CC1101-command strobes]----------------------------*/
#define SRES     0x30         // Reset chip
#define SFSTXON  0x31         // Enable/calibrate freq synthesizer
#define SXOFF    0x32         // Turn off crystal oscillator.
#define SCAL     0x33         // Calibrate freq synthesizer & disable
#define SRX      0x34         // Enable RX.
#define STX      0x35         // Enable TX.
#define SIDLE    0x36         // Exit RX / TX
#define SAFC     0x37         // AFC adjustment of freq synthesizer
#define SWOR     0x38         // Start automatic RX polling sequence
#define SPWD     0x39         // Enter pwr down mode when CSn goes hi
#define SFRX     0x3A         // Flush the RX FIFO buffer.
#define SFTX     0x3B         // Flush the TX FIFO buffer.
#define SWORRST  0x3C         // Reset real time clock.
#define SNOP     0x3D         // No operation.
/*-------------------------[END command strobes]------------------------------*/

/*----------------------[CC1101 - status register]----------------------------*/
#define PARTNUM        0xF0   // Part number
#define VERSION        0xF1   // Current version number
#define FREQEST        0xF2   // Frequency offset estimate
#define LQI            0xF3   // Demodulator estimate for link quality
#define RSSI           0xF4   // Received signal strength indication
#define MARCSTATE      0xF5   // Control state machine state
#define WORTIME1       0xF6   // High byte of WOR timer
#define WORTIME0       0xF7   // Low byte of WOR timer
#define PKTSTATUS      0xF8   // Current GDOx status and packet status
#define VCO_VC_DAC     0xF9   // Current setting from PLL cal module
#define TXBYTES        0xFA   // Underflow and # of bytes in TXFIFO
#define RXBYTES        0xFB   // Overflow and # of bytes in RXFIFO
#define RCCTRL1_STATUS 0xFC   //Last RC Oscillator Calibration Result
#define RCCTRL0_STATUS 0xFD   //Last RC Oscillator Calibration Result
//--------------------------[END status register]-------------------------------

/*----------------------[CC1101 - Main Radio Control State Machine states]-----*/
#define MARCSTATE_BITMASK          0x1F
#define MARCSTATE_SLEEP            0x00
#define MARCSTATE_IDLE             0x01
#define MARCSTATE_XOFF             0x02
#define MARCSTATE_VCOON_MC         0x03
#define MARCSTATE_REGON_MC         0x04
#define MARCSTATE_MANCAL           0x05
#define MARCSTATE_VCOON            0x06
#define MARCSTATE_REGON            0x07
#define MARCSTATE_STARTCAL         0x08
#define MARCSTATE_BWBOOST          0x09
#define MARCSTATE_FS_LOCK          0x0A
#define MARCSTATE_IFADCON          0x0B
#define MARCSTATE_ENDCAL           0x0C
#define MARCSTATE_RX               0x0D
#define MARCSTATE_RX_END           0x0E
#define MARCSTATE_RX_RST           0x0F
#define MARCSTATE_TXRX_SWITCH      0x10
#define MARCSTATE_RXFIFO_OVERFLOW  0x11
#define MARCSTATE_FSTXON           0x12
#define MARCSTATE_TX               0x13
#define MARCSTATE_TX_END           0x14
#define MARCSTATE_RXTX_SWITCH      0x15
#define MARCSTATE_TXFIFO_UNDERFLOW 0x16

// Chip Status Byte
// Bit fields in the chip status byte
#define CHIPSTATUS_CHIP_RDYn_BITMASK 0x80
#define CHIPSTATUS_STATE_BITMASK   0x70
#define CHIPSTATUS_FIFO_BYTES_AVAILABLE_BITMASK 0x0F
// Chip states
 #define CHIPSTATUS_STATE_IDLE 0x00
 #define CHIPSTATUS_STATE_RX 0x10
 #define CHIPSTATUS_STATE_TX 0x20
 #define CHIPSTATUS_STATE_FSTXON 0x30
 #define CHIPSTATUS_STATE_CALIBRATE 0x40
 #define CHIPSTATUS_STATE_SETTLING 0x50
 #define CHIPSTATUS_STATE_RX_OVERFLOW 0x60
 #define CHIPSTATUS_STATE_TX_UNDERFLOW 0x70

// loop states
#define RX_START 0
#define RX_ACTIVE 1
#define RX_END 2
#define TX_START 3
#define TX_ACTIVE 4
#define TX_END 5

class RfDataLinkLayer;

class RfPhysicalLayerCC1101 : public RfPhysicalLayer
{
  public:
    RfPhysicalLayerCC1101(RfDataLinkLayer& rfDataLinkLayer, Platform& platform);

    bool InitChip();
    void showRegisterSettings();
    void stopChip();
    void loop();

  private:
    // Table for encoding 4-bit data into a 8-bit Manchester encoding.
    static const uint8_t manchEncodeTab[16];
    // Table for decoding 4-bit Manchester encoded data into 2-bit
    static const uint8_t manchDecodeTab[16];

    static const uint8_t cc1101_2FSK_32_7_kb[CFG_REGISTER];
    static const uint8_t paTablePower868[8];

    void manchEncode(uint8_t *uncodedData, uint8_t *encodedData);
    bool manchDecode(uint8_t *encodedData, uint8_t *decodedData);

    void powerDownCC1101();
    void setOutputPowerLevel(int8_t dBm);

    uint8_t sIdle();
    uint8_t sReceive();

    void spiWriteRegister(uint8_t spi_instr, uint8_t value);
    uint8_t spiReadRegister(uint8_t spi_instr);
    uint8_t spiWriteStrobe(uint8_t spi_instr);
    void spiReadBurst(uint8_t spi_instr, uint8_t *pArr, uint8_t len);
    void spiWriteBurst(uint8_t spi_instr, const uint8_t *pArr, uint8_t len);

    uint8_t _loopState = RX_START;

    bool syncStart = false;
    bool packetStart = true;
    bool fixedLengthMode = false;
    uint8_t *sendBuffer {0};
    uint16_t sendBufferLength {0};
    uint8_t packet[512];
    uint8_t buffer[sizeof(packet)*2]; // We need twice the space due to manchester encoding
    uint8_t* pByteIndex = &buffer[0];
    uint16_t pktLen {0};
    uint16_t bytesLeft = {0};
    uint8_t statusGDO0 {0};
    uint8_t statusGDO2 {0};
    uint8_t prevStatusGDO0 {0}; // for edge detection during polling
    uint8_t prevStatusGDO2 {0}; // for edge detection during polling
    uint32_t packetStartTime {0};
};

#endif // USE_RF

#endif // DeviceFamily_CC13X0