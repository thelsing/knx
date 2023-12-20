#pragma once

#include "knx/bits.h"
#include "knx/config.h"
#include "knx/bau07B0.h"
#include "knx/bau091A.h"
#include "knx/bau27B0.h"
#include "knx/bau2920.h"
#include "knx/bau57B0.h"

#ifndef USERDATA_SAVE_SIZE
#define USERDATA_SAVE_SIZE 0
#endif

#ifdef ARDUINO_ARCH_SAMD
    #include "samd_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_RP2040)
    #include "rp2040_arduino_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_ESP8266)
    #include "esp_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_ESP32)
#if !defined(LED_BUILTIN)
    #define LED_BUILTIN 13
#endif
    #include "esp32_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif defined(ARDUINO_ARCH_STM32)
    #include "stm32_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        void buttonUp();
    #endif
#elif __linux__
#if !defined(LED_BUILTIN)
    #define LED_BUILTIN 0
#endif
    #include "linux_platform.h"
#else
#if !defined(LED_BUILTIN)
    #define LED_BUILTIN 5 // see GPIO_PinConfig gpioPinConfigs[]
#endif
    #include "cc1310_platform.h"
    #ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
        extern void buttonUp();
    #endif
#endif

#ifndef KNX_LED
    #define KNX_LED LED_BUILTIN
#endif
#ifndef KNX_LED_ACTIVE_ON
    #define KNX_LED_ACTIVE_ON 0
#endif
#ifndef KNX_BUTTON
    #define KNX_BUTTON -1
#endif

typedef const uint8_t* (*RestoreCallback)(const uint8_t* buffer);
typedef uint8_t* (*SaveCallback)(uint8_t* buffer);
typedef void (*IsrFunctionPtr)();
typedef void (*ProgLedOnCallback)();
typedef void (*ProgLedOffCallback)();

template <class P, class B> class KnxFacade : private SaveRestore
{
  public:
    KnxFacade() : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
    {
        manufacturerId(0xfa);
        bauNumber(platform().uniqueSerialNumber());
        _bau.addSaveRestore(this);
    }

    KnxFacade(B& bau) : _bau(bau)
    {
        _platformPtr = static_cast<P*>(&bau.platform());
        manufacturerId(0xfa);
        bauNumber(platform().uniqueSerialNumber());
        _bau.addSaveRestore(this);
    }

    KnxFacade(IsrFunctionPtr buttonISRFunction) : _platformPtr(new P()), _bauPtr(new B(*_platformPtr)), _bau(*_bauPtr)
    {
        manufacturerId(0xfa);
        bauNumber(platform().uniqueSerialNumber());
        _bau.addSaveRestore(this);
        setButtonISRFunction(buttonISRFunction);
    }

    virtual ~KnxFacade()
    {
        if (_bauPtr)
            delete _bauPtr;

        if (_platformPtr)
            delete _platformPtr;
    }

    P& platform()
    {
        return *_platformPtr;
    }

    B& bau()
    {
        return _bau;
    }

    bool enabled()
    {
        return _bau.enabled();
    }

    void enabled(bool value)
    {
        _bau.enabled(value);
    }

    bool progMode()
    {
        return _bau.deviceObject().progMode();
    }

    void progMode(bool value)
    {
        _bau.deviceObject().progMode(value);
    }

    /**
     * To be called by ISR handling on button press.
     */
    void toggleProgMode()
    {
        _toggleProgMode = true;
    }

    bool configured()
    {
        return _bau.configured();
    }

    /**
     * returns HIGH if led is active on HIGH, LOW otherwise
     */
    uint32_t ledPinActiveOn()
    {
        return _ledPinActiveOn;
    }

    /**
     * Sets if the programming led is active on HIGH or LOW. 
     * 
     * Set to HIGH for GPIO--RESISTOR--LED--GND or to LOW for GPIO--LED--RESISTOR--VDD
     */
    void ledPinActiveOn(uint32_t value)
    {
        _ledPinActiveOn = value;
    }

    uint32_t ledPin()
    {
        return _ledPin;
    }

    void ledPin(uint32_t value)
    {
        _ledPin = value;
    }

    void setProgLedOffCallback(ProgLedOffCallback progLedOffCallback)
    {
        _progLedOffCallback = progLedOffCallback;
    }

    void setProgLedOnCallback(ProgLedOnCallback progLedOnCallback)
    {
        _progLedOnCallback = progLedOnCallback;
    }

  
    int32_t buttonPin()
    {
        return _buttonPin;
    }

    void buttonPin(int32_t value)
    {
        _buttonPin = value;
    }

    void readMemory()
    {
        _bau.readMemory();
    }

    void writeMemory()
    {
        _bau.writeMemory();
    }

    uint16_t individualAddress()
    {
        return _bau.deviceObject().individualAddress();
    }

    void loop()
    {
        if (progMode() != _progLedState)
        {
            _progLedState = progMode();
            if (_progLedState)
            {
                println("progmode on");
                progLedOn();
            }
            else
            {
                println("progmode off");
                progLedOff();
            }
        }
        if (_toggleProgMode)
        {
            progMode(!progMode());
            _toggleProgMode = false;
        }
        _bau.loop();
    }

    void manufacturerId(uint16_t value)
    {
        _bau.deviceObject().manufacturerId(value);
    }

    void bauNumber(uint32_t value)
    {
        _bau.deviceObject().bauNumber(value);
    }
    
    void orderNumber(const uint8_t* value)
    {
        _bau.deviceObject().orderNumber(value);
    }

    void hardwareType(const uint8_t* value)
    {
        _bau.deviceObject().hardwareType(value);
    }
    
    void version(uint16_t value)
    {
        _bau.deviceObject().version(value);
    }

    void start()
    {
        if (_progLedOffCallback == 0 || _progLedOnCallback == 0)
            pinMode(ledPin(), OUTPUT);

        progLedOff();
        pinMode(buttonPin(), INPUT_PULLUP);

        if (_progButtonISRFuncPtr && _buttonPin >= 0)
        {
            // Workaround for https://github.com/arduino/ArduinoCore-samd/issues/587
            #if (ARDUINO_API_VERSION >= 10200)
                attachInterrupt(_buttonPin, _progButtonISRFuncPtr, (PinStatus)CHANGE);
            #else
                attachInterrupt(_buttonPin, _progButtonISRFuncPtr, CHANGE);
            #endif
        }

        enabled(true);
    }

    void setButtonISRFunction(IsrFunctionPtr progButtonISRFuncPtr)
    {
        _progButtonISRFuncPtr = progButtonISRFuncPtr;
    }

    void setSaveCallback(SaveCallback func)
    {
        _saveCallback = func;
    }

    void setRestoreCallback(RestoreCallback func)
    {
        _restoreCallback = func;
    }

    uint8_t* paramData(uint32_t addr)
    {
        if (!_bau.configured())
            return nullptr;

        return _bau.parameters().data(addr);
    }

    // paramBit(address, shift)
    // get state of a parameter as a boolean like "enable/disable", ...
    // Declaration in XML file:
    // ...
    // <ParameterType Id="M-00FA_A-0066-EA-0001_PT-toggle" Name="toggle">
    //   <TypeRestriction Base="Value" SizeInBit="1">
    //     <Enumeration Text="Désactivé" Value="0" Id="M-00FA_A-0066-EA-0001_PT-toggle_EN-0"/>
    //     <Enumeration Text="Activé" Value="1" Id="M-00FA_A-0066-EA-0001_PT-toggle_EN-1"/>
    //  </TypeRestriction>
    // </ParameterType>
    // ...
    // <Parameter Id="M-00FA_A-0066-EA-0001_P-2" Name="Input 1" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 1" Value="1">
    //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="0"/>
    // </Parameter>
    // <Parameter Id="M-00FA_A-0066-EA-0001_P-3" Name="Input 2" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 2" Value="1">
    //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="1"/>
    // </Parameter>
    // <Parameter Id="M-00FA_A-0066-EA-0001_P-4" Name="Inout 3" ParameterType="M-00FA_A-0066-EA-0001_PT-toggle" Text="Input 3" Value="1">
    //   <Memory CodeSegment="M-00FA_A-0066-EA-0001_RS-04-00000" Offset="1" BitOffset="2"/>
    // </Parameter>
    // ...
    // Usage in code :
    //   if ( knx.paramBit(1,1))
    //   {
    //      //do somthings ....
    //   }
    bool paramBit(uint32_t addr, uint8_t shift)
    {
        if (!_bau.configured())
            return 0;
   
        return (bool) ((_bau.parameters().getByte(addr) >> (7-shift)) & 0x01); 
    }

    uint8_t paramByte(uint32_t addr)
    {
        if (!_bau.configured())
            return 0;

        return _bau.parameters().getByte(addr);
    }
    
    // Same usage than paramByte(addresse) for signed parameters
    // Declaration in XML file
    // <ParameterType Id="M-00FA_A-0066-EA-0001_PT-delta" Name="delta">
    //   <TypeNumber SizeInBit="8" Type="signedInt" minInclusive="-10" maxInclusive="10"/>
    // </ParameterType>
    int8_t paramSignedByte(uint32_t addr)
    {
        if (!_bau.configured())
            return 0;

        return (int8_t) _bau.parameters().getByte(addr);
    }
 
    uint16_t paramWord(uint32_t addr)
    {
        if (!_bau.configured())
            return 0;

        return _bau.parameters().getWord(addr);
    }

    uint32_t paramInt(uint32_t addr)
    {
        if (!_bau.configured())
            return 0;

        return _bau.parameters().getInt(addr);
    }

    double paramFloat(uint32_t addr, ParameterFloatEncodings enc)
    {
        if (!_bau.configured())
            return 0;

        return _bau.parameters().getFloat(addr, enc);
    }
    
#if (MASK_VERSION == 0x07B0) || (MASK_VERSION == 0x27B0) || (MASK_VERSION == 0x57B0)
    GroupObject& getGroupObject(uint16_t goNr)
    {
        return _bau.groupObjectTable().get(goNr);
    }
#endif

    void restart(uint16_t individualAddress)
    {
        SecurityControl sc = {false, None};
        _bau.restartRequest(individualAddress, sc);
    }

    void beforeRestartCallback(BeforeRestartCallback func)
    {
        _bau.beforeRestartCallback(func);
    }

    BeforeRestartCallback beforeRestartCallback()
    {
        return _bau.beforeRestartCallback();
    }

  private:
    P* _platformPtr = 0;
    B* _bauPtr = 0;
    B& _bau;
    ProgLedOnCallback _progLedOnCallback = 0;
    ProgLedOffCallback _progLedOffCallback = 0;
    uint32_t _ledPinActiveOn = KNX_LED_ACTIVE_ON;
    uint32_t _ledPin = KNX_LED;
    int32_t _buttonPin = KNX_BUTTON;
    SaveCallback _saveCallback = 0;
    RestoreCallback _restoreCallback = 0;
    volatile bool _toggleProgMode = false;
    bool _progLedState = false;
    uint16_t _saveSize = USERDATA_SAVE_SIZE;
    IsrFunctionPtr _progButtonISRFuncPtr = 0;

    uint8_t* save(uint8_t* buffer)
    {
        if (_saveCallback != 0)
            return _saveCallback(buffer);

        return buffer;
    }

    const uint8_t* restore(const uint8_t* buffer)
    {
        if (_restoreCallback != 0)
            return _restoreCallback(buffer);

        return buffer;
    }

    uint16_t saveSize()
    {
        return _saveSize;
    }

    void saveSize(uint16_t size)
    {
        _saveSize = size;
    }

    void progLedOn()
    {
        if (_progLedOnCallback == 0)
            digitalWrite(ledPin(), _ledPinActiveOn);
        else
            _progLedOnCallback();
    }

    void progLedOff()
    {
        if (_progLedOffCallback == 0)
            digitalWrite(ledPin(), HIGH - _ledPinActiveOn);
        else
            _progLedOffCallback();
    }
};

#ifndef KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
    #ifdef ARDUINO_ARCH_SAMD
        // predefined global instance for TP or RF or TP/RF coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<SamdPlatform, Bau07B0> knx;
        #elif MASK_VERSION == 0x27B0
            extern KnxFacade<SamdPlatform, Bau27B0> knx;
        #elif MASK_VERSION == 0x2920
            extern KnxFacade<SamdPlatform, Bau2920> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_SAMD"
        #endif
    #elif defined(ARDUINO_ARCH_RP2040)
        // predefined global instance for TP or RF or TP/RF or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<RP2040ArduinoPlatform, Bau07B0> knx;
        #elif MASK_VERSION == 0x27B0
            extern KnxFacade<RP2040ArduinoPlatform, Bau27B0> knx;
        #elif MASK_VERSION == 0x57B0
            extern KnxFacade<RP2040ArduinoPlatform, Bau57B0> knx;
        #elif MASK_VERSION == 0x2920
            extern KnxFacade<RP2040ArduinoPlatform, Bau2920> knx;
        #elif MASK_VERSION == 0x091A
            extern KnxFacade<RP2040ArduinoPlatform, Bau091A> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_RP2040"
        #endif
    #elif defined(ARDUINO_ARCH_ESP8266)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<EspPlatform, Bau07B0> knx;
        #elif MASK_VERSION == 0x57B0
            extern KnxFacade<EspPlatform, Bau57B0> knx;
        #elif MASK_VERSION == 0x091A
            extern KnxFacade<EspPlatform, Bau091A> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP8266"
        #endif
    #elif defined(ARDUINO_ARCH_ESP32)
        // predefined global instance for TP or IP or TP/IP coupler
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<Esp32Platform, Bau07B0> knx;
        #elif MASK_VERSION == 0x57B0
            extern KnxFacade<Esp32Platform, Bau57B0> knx;
        #elif MASK_VERSION == 0x091A
            extern KnxFacade<Esp32Platform, Bau091A> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_ESP32"
        #endif
    #elif defined(ARDUINO_ARCH_STM32)
        // predefined global instance for TP only
        #if MASK_VERSION == 0x07B0
            extern KnxFacade<Stm32Platform, Bau07B0> knx;
        #else
            #error "Mask version not supported on ARDUINO_ARCH_STM32"
        #endif
    #else // Non-Arduino platforms and Linux platform
        // no predefined global instance
    #endif
#endif // KNX_NO_AUTOMATIC_GLOBAL_INSTANCE
