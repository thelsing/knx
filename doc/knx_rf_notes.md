KNX-RF S-Mode
=============

Implementation Notes
--------------------
* KNX-RF E-Mode (pushbutton method) is NOT supported!
* KNX-RF S-Mode is implemented as KNX-RF READY (KNX-RF 1.R) which means only one single channel and no fast-ack is used.
  * KNX RF Multi (KNX-RF 1.M) would be required for this. However, implementation is way more complex as frequency hopping (fast and slow channels) is used and fast-ack
     is based on a TDMA-like access scheme which has very strict timing requirements for the time slots.
     
  * summary: KNX-RF 1.R does NOT acknowledge packets on the air (data link layer). So standard GROUP_VALUE_WRITE messages in multicast mode could get lost!
              Connection-oriented communication for device management is handled by the transport layer and uses T_ACK and T_NACK.
* the driver (rf_physical_layer.cpp) uses BOTH signals (GDO0, GDO2) of the RF transceiver C1101
  * GDO0 is asserted on RX/TX packet start and de-asserted on RX/TX packet end or RX/TX FIFO overflow/underflow
  * GDO2 is asserted is the FIFO needs to read out (RX) or refilled (TX)
  
* the driver (rf_physical_layer.cpp) uses both packet length modes of the CC1101: infinite length and fixed length are switched WHILE receiving or transmitting a packet.

* the edges of the signals GDO0 and GDO2 are detected by polling in the main loop and NOT by using interrupts
  * as a consequence the main loop must not be delayed too much, the transceiver receives/transmitts the data bytes itself into/from the FIFOs though (max. FIFO size 64 bytes each (TX/RX)!).
  * KNX-RF bitrate is 16384 bits/sec. -> so 40 bytes (Preamble, Syncword, Postamble) around 20ms for reception/transmission of a complete packet (to be verified!)
  * another implementation using interrupts could also be realized
  
* the driver does not use the wake-on-radio of the CC1101. The bidirectional battery powered devices are not useful at the moment.
  * wake-on-radio would also require the MCU to sleep and be woken up through interrupts.
  * BUT: UNIdirectional (battery powered) device could be realized: one the device has been configured in bidirectional mode.
          The device (MCU) could sleep and only wake up if something needs to be done. Only useful for transmitters (e.g. sensors like push button, temperature sensor, etc.).

ToDo
----
* Packet duplication prevention based on the data link layer frame counter if KNX-RF retransmitters (range extension) are active (should be easy to add)
* KNX-RF 1.M (complex, may need an additional MCU, not planned for now)
  * maybe with a more capable transceiver: http://www.lapis-semi.com/en/semicon/telecom/telecom-product.php?PartNo=ML7345
     it could handle manchester code, CRC-16 and the whole Wireless MBUS frame structure in hardware. Also used by Tapko for KAIstack.
* KNX Data Secure with security proxy profile in line coupler (e.g. TP<>RF). See KNX AN158 (KNX Data Secure draft spec.) p.9
  * KNX-RF very much benefits from having authenticated, encrypted data exchange. 
  * Security Proxy terminates the secure applicationj layer (S-AL) in the line coupler. So the existing plain TP installation without data secure feature
     can be kept as is.

Development Setup
-----------------
* Development is done on a cheap Wemos SAMD21 M0-Mini board with a standard CC1101 module (868MHz) from Ebay. 

* Beware of defective and bad quality modules!

* The SAMD21 MCU is connected via SWD (Segger J-Link) to PlatformIO (Visual Studio Code). Additionally the standard UART (Arduino) is used for serial debug messages.

* The USB port of the SAMD21 is not used at all.

Connection wiring:
------------------

Signal    |   SAMD21    |   CC1101    
----------|-------------|--------------
SPI_nCS   |  D10(PA18)  |     CSN
SPI_MOSI  |  D11(PA16)  |     SI
SPI_MISO  |  D12(PA19)  |     SO
SPI_SCK   |  D13(PA17)  |     SCLK
GDO0      |  D7(PA21)   |     GDO0
GDO2      |  D9(PA07)   |     GDO2

Arduino MZEROUSB variant needs patching to enable SPI on SERCOM1 on D10-D13.

If you do not want to patch variant files, use a compatible board that provides SPI on D10-D13.

variant.h
---------
```
/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO         (18u)
#define PIN_SPI_MOSI         (21u)
#define PIN_SPI_SCK          (20u)
#define PERIPH_SPI           sercom1
#define PAD_SPI_TX           SPI_PAD_0_SCK_1
#define PAD_SPI_RX           SERCOM_RX_PAD_3
```

variant.cpp
-----------
```
  // 18..23 - SPI pins (ICSP:MISO,SCK,MOSI)
  // ----------------------
  { PORTA, 19, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_12 }, // MISO: SERCOM1/PAD[3]
  { NOT_A_PORT, 0, PIO_NOT_A_PIN, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // 5V0
  { PORTA, 17, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_11 }, // SCK: SERCOM1/PAD[1]
  { PORTA, 16, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_10 }, // MOSI: SERCOM1/PAD[0]
  { NOT_A_PORT, 0, PIO_NOT_A_PIN, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // RESET
  { NOT_A_PORT, 0, PIO_NOT_A_PIN, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // GND
```
