#include <knx.h>
#include <PZEM004Tv30.h>
#include "wiring_private.h" // pinPeripheral() function


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

//Blinking live LED
const uint8_t ledPin =  LED_BUILTIN;// the number of the LED pin
bool ledState = LOW;             // ledState used to set the LED
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 2000;           // interval at which to blink (milliseconds)

//PZEM stuff
#define PZEM004_NO_SWSERIAL
#define PZEM_DEFAULT_ADDR 0xF8

const uint8_t physicalCount = 6; // voltage,current,power_factor,power,energy,frequency

//knx stuff
#define goReset knx.getGroupObject(1)
#define goDateTime knx.getGroupObject(2)

const uint8_t ets_startupTimeout[7] = {0, 1, 2, 3, 4, 5, 6};
const int ets_timePeriod[7] = {0, 1, 5, 15, 1 * 60, 5 * 60, 15 * 60};
const uint8_t ets_percentCycle[6] = {0, 5, 10, 15, 20, 30}; //need knxprod update... ?

int percentCycle = 0; // better to define a global or read knx.paramByte each time... ?
unsigned long timePeriod = 0; // same here,
uint8_t resetFlag = 0;    // and here...

// Issue on https://github.com/mandulaj/PZEM-004T-v30/issues/43
PZEM004Tv30 pzem(Serial2, PZEM_DEFAULT_ADDR);


struct Physical {
    void init(uint8_t GOaddr, Dpt type_dpt){
      _GOaddr = GOaddr;
      _dpt = type_dpt;
    }
  
    void loop(float value){
      unsigned long currentMillis = millis();
      
      // Delta Change update as defined in ETS

      int deltaPercent = ( 100 * ( value - lastValue ) / value );
      if ( percentCycle != 0 && abs(deltaPercent) >= percentCycle )
      {
          trigger = true;
      }

      // Refresh groupAddress value as defined in ETS since last update
      if ( timePeriod != 0 && currentMillis - lastUpdate >= timePeriod )
      {
          trigger = true;
      }

      // UpdateGO but send to bus only if triggered by time or value change percentage
      if (trigger){
          knx.getGroupObject(_GOaddr).value(value, _dpt);
          lastUpdate = millis();
          trigger = false;
      }else{
          knx.getGroupObject(_GOaddr).valueNoSend(value, _dpt);
      }
      lastValue = value;
    }

  private:
    uint8_t _GOaddr;
    Dpt _dpt;
    bool trigger = false;
//    bool isUpdated = false;

  public:
    float lastValue = 0;
    unsigned long lastUpdate = 0;
  
} Physical[physicalCount];


// callback from reset-GO
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

  SerialUSB.begin(9600);
  Serial2.begin(9600);

  ArduinoPlatform::SerialDebug = &SerialUSB;

  randomSeed(millis());

  knx.readMemory();

    if (knx.configured())
    {
        int confStartupTime = ets_startupTimeout[knx.paramByte(0)] * 1000;
        delay(confStartupTime); // the only delay used, why make a withoutDelay function for that?

        percentCycle = ets_percentCycle[knx.paramByte(1)];
        timePeriod = ets_timePeriod[knx.paramByte(2)] * 1000;
        
        resetFlag = knx.paramByte(3);

        goReset.callback(resetCallback);
        goReset.dataPointType(DPT_Trigger);
        goDateTime.dataPointType(DPT_DateTime);

        uint8_t GOaddr = 3;
        Physical[0].init(GOaddr, DPT_Value_Electric_Potential); // voltage
        Physical[1].init(GOaddr += 1, DPT_Value_Electric_Current);
        Physical[2].init(GOaddr += 1, DPT_Value_Power_Factor);
        Physical[3].init(GOaddr += 1, DPT_Value_Power);
        Physical[4].init(GOaddr += 1, DPT_Value_Energy);
        Physical[5].init(GOaddr += 1, DPT_Value_Frequency);
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

    if (!knx.configured()) {
      return;
    }
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) { // I love blinking LED state...
        previousMillis = currentMillis;
        if (ledState == LOW) {
        ledState = HIGH;
        } else {
        ledState = LOW;
        }
        digitalWrite(ledPin, ledState);
    }
    for (uint8_t i=0; i< physicalCount; i++)
    {
        if (currentMillis - Physical[i].lastUpdate >= interval)
        {
            float isanValue = refreshValue(i);
            if(!isnan(isanValue))
            {
                Physical[i].loop(isanValue);
            }
        }
    }
}

float refreshValue(uint8_t physicalNumber){
    float valueTemp;
    switch (physicalNumber) {           //maybe a pointer or reference could be nicer...
        case 0:
            valueTemp = pzem.voltage();
            return valueTemp;
        case 1:
            valueTemp = pzem.current();
            return valueTemp;
        case 2:
            valueTemp = pzem.pf();
            return valueTemp;
        case 3:
            valueTemp = pzem.power();
            return valueTemp;
        case 4:
            valueTemp = pzem.energy();
            return valueTemp;
        case 5:
            valueTemp = pzem.frequency();
            return valueTemp;
        default:
            break;
        }
}

void resetEnergy(){
   pzem.resetEnergy();
}