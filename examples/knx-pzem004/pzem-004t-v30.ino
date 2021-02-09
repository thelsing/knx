#include <knx.h>
#include <PZEM004Tv30.h>
#include "wiring_private.h" // pinPeripheral() function

#include <TimeLib.h>

//Sercom Stuff
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12 (34ul)
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10 (36ul)
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX); //TX D10, RX D12

void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

//PZEM stuff
#define PZEM004_NO_SWSERIAL
#define PZEM_DEFAULT_ADDR 0xF8


//knx stuff
#define goReset knx.getGroupObject(1)
#define goDateTime knx.getGroupObject(2)
#define goProgMode knx.getGroupObject(9)

// Global Const
const uint16_t ets_timePeriod[7] = {0, 1, 5, 15, 1 * 60, 5 * 60, 15 * 60};
const uint8_t ets_startupTimeout[7] = {0, 1, 2, 3, 4, 5, 6};
const uint8_t ets_percentCycle[6] = {0, 5, 10, 15, 20, 30}; //need knxprod update... ?

const uint8_t ledPin =  LED_BUILTIN;// the number of the LED pin
const uint8_t physicalCount = 6; // voltage,current,power_factor,power,energy,frequency

// Global Variable
uint8_t percentCycle = 0; // better to define a global or read knx.paramByte each time... ?
uint32_t timePeriod = 0; // same here,
uint8_t resetPeriod = 0; //same here ...
//uint8_t resetEnergy = 0;    // and here... disabled/day/week/month

bool progMode = true;

// Issue on https://github.com/mandulaj/PZEM-004T-v30/issues/43
PZEM004Tv30 pzem(Serial2, PZEM_DEFAULT_ADDR);

struct Physical {
    void init(uint8_t GOaddr, Dpt type_dpt){
      _GOaddr = GOaddr;
      _dpt = type_dpt;
    }
  
    void loop(){
//      unsigned long currentMillis = millis();
      // Delta Change update as defined in ETS
      float deltaPercent = ( 100 * ( _value - _lastValue ) / _value );
      if ( percentCycle != 0 && abs(deltaPercent) >= percentCycle )
      {
          _trigger = true;
      }

      // Refresh groupAddress value as defined in ETS since last update
      if ( timePeriod != 0 && millis() - _lastMillis >= timePeriod )
      {
          _trigger = true;
      }

      // UpdateGO but send to bus only if triggered by time or value change percentage
      if (_trigger){
          knx.getGroupObject(_GOaddr).value(_value, _dpt);
          _lastValue = _value;
          _lastMillis = millis();
          _trigger = false;
      }else{
          knx.getGroupObject(_GOaddr).valueNoSend(_value, _dpt);
      }
    }

    void setValue(float value){
        if (value != _value)
        {
            _value = value;
        }
    }

  private:
    Dpt _dpt;
    float _value = 0;
    float _lastValue = 0;
    uint32_t _lastMillis = 0;
    uint8_t _GOaddr;
    bool _trigger = false;

//    bool isUpdated = false;

  public:
  
} Physical[physicalCount];


class Blinker
{
  private:
    uint8_t ledPin_;      // the number of the LED pin
    uint32_t OnTime = 1000;     // milliseconds of on-time
    uint32_t OffTime = 1000;    // milliseconds of off-time
    bool ledState = LOW;                 // ledState used to set the LED
    uint32_t previousMillis;   // will store last time LED was updated

    void setOutput(bool state_, uint32_t currentMillis_){
        ledState = state_;
        previousMillis = currentMillis_;
        digitalWrite(ledPin_, state_);
    }

  public:
    Blinker(uint8_t pin)
    {
        ledPin_ = pin;
        pinMode(ledPin_, OUTPUT);
        previousMillis = 0;
    }
  
    void set(uint32_t on, uint32_t off){
        OnTime = on;
        OffTime = off;
    }
  
    void loop(){
        uint32_t currentMillis = millis();
         
        if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
        {
            setOutput(LOW, currentMillis);
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
        {
            setOutput(HIGH, currentMillis);
        }
    }
};

Blinker led = Blinker(ledPin);

void callBackProgMode(GroupObject& go){ 
    progMode = (bool)go.value();
}

void callBackDateTime(GroupObject& go){
    static uint32_t lastUpdate = 0;
    const uint32_t interval = (1000 * 60 * 60 * 24); // 1day

    struct tm myTime;
    myTime = go.value();
    unsigned short tmp_sec = myTime.tm_sec;
    unsigned short tmp_min = myTime.tm_min;
    unsigned short tmp_hour = myTime.tm_hour;
    unsigned short tmp_mday = myTime.tm_mday;
    unsigned short tmp_month = myTime.tm_mon;
    unsigned short tmp_year = myTime.tm_year;

    if (millis() - lastUpdate >= interval && !timeStatus() == timeSet)
    {
        setTime(tmp_hour, tmp_min, tmp_sec, tmp_mday, tmp_month, tmp_year);
        lastUpdate = millis();
    }
}

void resetCallback(GroupObject& go)
{
    if (go.value())
    {
        pzem.resetEnergy();
        goReset.value(false);
    }
}

void setup() {
  pinPeripheral(PIN_SERIAL2_RX, PIO_SERCOM);
  pinPeripheral(PIN_SERIAL2_TX, PIO_SERCOM);

//  SerialUSB.begin(9600);
  Serial2.begin(9600);

  ArduinoPlatform::SerialDebug = &SerialUSB;

  randomSeed(millis());

  knx.readMemory();
//   led.set(5000, 5000);

    if (knx.configured())
    {
        int confStartupTime = ets_startupTimeout[knx.paramByte(0)] * 1000;
        delay(confStartupTime); // the only delay used, why make a withoutDelay function for that?

        percentCycle = ets_percentCycle[knx.paramByte(1)];
        timePeriod = ets_timePeriod[knx.paramByte(2)] * 1000;
        
        resetPeriod = knx.paramByte(3);

        goReset.callback(resetCallback);
        goReset.dataPointType(DPT_Trigger);
        
        goDateTime.dataPointType(DPT_DateTime);

        goProgMode.dataPointType(DPT_Trigger);
        goProgMode.callback(callBackProgMode);

        uint8_t GOaddr = 3;
        Physical[0].init(GOaddr, DPT_Value_Electric_Potential); // voltage
        Physical[1].init(GOaddr += 1, DPT_Value_Electric_Current);
        Physical[2].init(GOaddr += 1, DPT_Value_Power_Factor);
        Physical[3].init(GOaddr += 1, DPT_Value_Power);
        Physical[4].init(GOaddr += 1, DPT_Value_Energy);
        Physical[5].init(GOaddr += 1, DPT_Value_Frequency);
        led.set(2000, 1000);
    }

    // is the led active on HIGH or low? Default is LOW
    knx.ledPinActiveOn(HIGH);
    // pin or GPIO programming button is connected to. Default is 0
    knx.ledPin(5);
    knx.buttonPin(9);

    knx.start();
//  while (!SerialUSB) { //wait for DEBUGING
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
}

void loop() {
    
    knx.loop();

    if (knx.configured() && !progMode)
    {
        refreshValueLoop();

        for (uint8_t i=0; i< physicalCount; i++)
        {
             Physical[i].loop();
        }

        if (timeStatus() == timeSet && resetPeriod != 0)
        {
           resetEnergyLoop();
        }
    
    }
    else if (progMode)
    {
        prodModeLoop();
    }
}

void refreshValueLoop(){
    static const uint16_t pzemInterval = 500;           // interval at which to blink (milliseconds)
    static uint32_t lastPzemUpdate = 0;

    if (millis() - lastPzemUpdate >= pzemInterval)
    {
        for (uint8_t i=0; i < physicalCount; i++)
        {
            float isaValue;
            switch (i) {           //maybe a pointer or reference could be nicer...
                case 0:
                    isaValue = pzem.voltage();
                    break;
                case 1:
                    isaValue = pzem.current();
                    break;
                case 2:
                    isaValue = pzem.pf();
                    break;
                case 3:
                    isaValue = pzem.power();
                    break;
                case 4:
                    isaValue = pzem.energy();
                    break;
                case 5:
                    isaValue = pzem.frequency();
                    break;
                default:
                    break;
            }
            
            if(!isnan(isaValue))
            {
                Physical[i].setValue(isaValue);
            }
            else
            {
                Physical[i].setValue(-1);
            }
        }
        lastPzemUpdate = millis();
        led.set(500, 1000);
    }
}

void resetEnergyLoop(){
    static time_t lastTime;
    time_t samdTime = now();

    if (timeStatus() == timeSet)
    {
        switch (resetPeriod)
        {
        case 1: //day
            if (day(samdTime) != day(lastTime))
            {
                lastTime = samdTime;
                pzem.resetEnergy();
            }
            break;
        case 2: //week
            if (weekday(samdTime) != weekday(lastTime) && weekday(samdTime) == 2) //monday
            {
                lastTime = samdTime;
                pzem.resetEnergy();
            }
            break;
        case 3: // month
            if (month(samdTime) != month(lastTime))
            {
                lastTime = samdTime;
                pzem.resetEnergy();
            }
            break;
        case 4: // year
            if (year(samdTime) != year(lastTime))
            {
                lastTime = samdTime;
                pzem.resetEnergy();
            }
        default:
            break;
        }
    }
}

void prodModeLoop(){ // run Only if progMode triggered ( at start or callback)
    static uint32_t timerProgPrevMillis = 0;
    const uint32_t timerProgMode = ( 15 * 60 * 1000 ) ; // 15min
    
    if (!knx.progMode())
    {
        knx.progMode(true);
        timerProgPrevMillis = millis();
        led.set(50, 100);
    }
    else
    {
        if (millis() - timerProgPrevMillis > timerProgMode) {
            knx.progMode(false);
            goProgMode.value(false);
            progMode = false;
        }
    }
}
