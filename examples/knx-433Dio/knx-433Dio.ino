#include <knx.h>
//#include <DiOremote.h>

//#define DEBUGSERIAL 1
#ifdef DEBUGSERIAL
  #define DPRINT(...)    SerialUSB.print(__VA_ARGS__)
  #define DPRINTLN(...)  SerialUSB.println(__VA_ARGS__)
  #include <MemoryFree.h>
  #include <pgmStrToRAM.h>
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

#define goButton1 knx.getGroupObject(1)
#define goButton2 knx.getGroupObject(2)
#define goButton3 knx.getGroupObject(3)
#define goButtonAll knx.getGroupObject(4)
#define goProgMode knx.getGroupObject(5)

//DiOremote myRemote = DiOremote(6);

//void function2();
//void function1(void (*)());
//void loop() {
//    function1(function2);
//}

//Global Const
const uint8_t ets_startupTimeout[7] = {0, 1, 2, 3, 4, 5, 6};
const uint16_t ets_timePeriod[7] = {0, 1, 5, 15, 1 * 60, 5 * 60, 15 * 60};
const uint8_t ets_progMode[7] = {0, 1, 2 * 60, 3 * 60, 4 * 60, 5 * 60, 10 * 60}; //need knxprod update... ?
const uint8_t ledPin =  LED_BUILTIN;
const uint8_t rfPin = 6;

//  //Protocol timing (in us)
//  #define DiOremote_START_FRAME_1 220
//  #define DiOremote_START_FRAME_0 2675
//  #define DiOremote_THIGH 220
//  #define DiOremote_TLOW_0 350 short
//  #define DiOremote_TLOW_1 1400 long
//  #define DiOremote_END_FRAME_1 220
//  #define DiOremote_END_FRAME_0 10600

// Global Variable
bool progMode = true;
// bool codeSendindBlock = false;

uint8_t percentCycle = 0; // better to define a global or read knx.paramByte each time... ?
uint32_t timePeriod = 0; // same here,
uint32_t timerProgMode = 0; // same here,
uint32_t ch1_on, ch1_off, ch2_on, ch2_off, ch3_on, ch3_off, chall_on, chall_off;

class RfCode {
  private: 
    uint8_t _rfPin;
    uint32_t _codeValueOn;
    uint32_t _codeValueOff;  
    uint8_t _GOaddress;
    const uint16_t  THIGH = 220, TSTART = 2675, TSHORT = 220, TLONG = 1400, TEND = 10600;
    uint8_t _loopCount = 0;  // fixed ==5
    bool lastState = false;
    uint32_t _codePending = 0;
    uint32_t _codePendingMemory = _codePending;
    uint8_t _loopPending = 0; // fixed ==32
    bool _sendMsgPending= false;

    enum PulseStates {
        PULSE_INIT,
        PULSE_KEY1, //32 times loop
        PULSE_KEY2, //32 times loop
        PULSE_END   
    };
    PulseStates pulseStates = PULSE_INIT;
    
    bool pulseSend(const uint16_t delayHigh, const uint16_t delayLow){ //alike state machine?

        static bool initPulse = false;
        static bool highDone = false;
        static uint32_t pulseLastTime = 0;
      
        uint32_t currentTime = micros();
        
        if (!initPulse){
          initPulse = true;
          digitalWrite(_rfPin, HIGH);
          pulseLastTime = currentTime;
        }
        else if (currentTime - pulseLastTime >= delayHigh && !highDone)
        {
          digitalWrite(_rfPin, LOW);
          pulseLastTime = currentTime;
          highDone = true;
        }
        else if (currentTime - pulseLastTime >=  delayLow && highDone)
        {
          initPulse = false;
          highDone = false;
        }
        return !initPulse;
    }

    uint16_t TTime(bool invert){
        uint16_t TTIME;
        
        if (_codePending & 0x80000000L)// future bug if uint32_t _codePengin > 2^32 / 2 ?
        {
            TTIME = invert ? TSHORT : TLONG;
        }
        else
        {
            TTIME = invert ? TLONG : TSHORT;
        }
        return TTIME;
    }
    
  public:
      RfCode(uint8_t rfPin){
        _rfPin = rfPin;
        pinMode(_rfPin, OUTPUT);
      }
      
//      void init(uint8_t addr, uint32_t codeValueOn, uint32_t codeValueOff){   //GroupObject &device, long timeOn){
//          _codeValueOn = codeValueOn;
//          _codeValueOff = codeValueOff;
//          _GOaddress = addr;
//      }

      void setState(bool modeOnOff, uint32_t codeValueOn, uint32_t codeValueOff){
        if (!_sendMsgPending)
        {
            _sendMsgPending = true;
           _codeValueOn = codeValueOn;
           _codeValueOff = codeValueOff;
//           _GOaddress = addr;           
            
            if (modeOnOff){
                _codePending = _codeValueOn;
//                SerialUSB.println(_codePending);
            }
            else
            {
                _codePending = _codeValueOff;
//                SerialUSB.println(_codePending);
            }
            _codePendingMemory = _codePending;
        }
      }
      
//      bool getMsgPendingState(){
//          return _sendMsgPending;
//      }
      
      void loop(){
          if (_sendMsgPending)
          {
              // needed to block setState of another Class, yes FIFO is better...
//              codeSendindBlock = true;
              if (_loopCount < 5)
              {
                  switch (pulseStates){
                      case PULSE_INIT:
                          if (pulseSend(THIGH, TSTART)){
                              pulseStates = PULSE_KEY1;
                          }
                          break;
                      case PULSE_KEY1:              
                          if (pulseSend(THIGH, TTime(false)))
                          {
                              pulseStates = PULSE_KEY2;
                          }
                          break;
                      case PULSE_KEY2:
                          if (pulseSend(THIGH, TTime(true)))
                          {
                              if (_loopPending < 32){
                                  _codePending <<= 1;
                                  _loopPending++;
                                  pulseStates = PULSE_KEY1; // next loop !
                              }
                              else{ // finish!
                                  pulseStates = PULSE_END;
                              }
                          }
                          break;
                      case PULSE_END:
                          if (pulseSend(THIGH, TEND)){
                              _loopCount++;
                              _codePending = _codePendingMemory;
                              _loopPending = 0;
                              pulseStates = PULSE_INIT;
                          }
                          break;
                      default:
                          break;
                  } 
              }
              else
              {
                  _loopCount = 0;
                  _sendMsgPending = false;
//                  codeSendindBlock = false;
              }
          }        
      }
};


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

RfCode button = RfCode(5);

Blinker led = Blinker(ledPin);


void callBackButton1(GroupObject& go){
    button.setState((bool)go.value(), ch1_on, ch1_off);    
}
void callBackButton2(GroupObject& go){
    button.setState((bool)go.value(), ch2_on, ch2_off);    
}
void callBackButton3(GroupObject& go){
    button.setState((bool)go.value(), ch3_on, ch3_off);    
}
void callBackButtonAll(GroupObject& go){
    button.setState((bool)go.value(), chall_on, ch3_off);    
}
void callBackProgMode(GroupObject& go){ 
    progMode = (bool)go.value();
}

void setup() {

//    #ifdef DEBUGSERIAL
//    SerialUSB.begin(9600);
//    while (!SerialUSB) { //wait for DEBUGING
//        ; // wait for serial port to connect. Needed for native USB port only
//    }
//    ArduinoPlatform::SerialDebug = &SerialUSB;
//    #endif

    randomSeed(millis());
    // knx.bau().deviceObject().individualAddress(1);
    knx.readMemory();

    if (knx.configured())
    {
        DPRINT("Setup: KNX Configuration...");

        progMode = false; // don't need to put device in progMode.
        int confStartupTime = ets_startupTimeout[knx.paramByte(0)] * 1000;
        delay(confStartupTime); // the only delay used, why make a withoutDelay function for that?

        timePeriod = ets_timePeriod[knx.paramByte(1)] * 1000;
//        timerProgMode = ets_progMode[knx.paramByte(34)] * 1000;

        ch1_on =  knx.paramInt(2);
        ch1_off =  knx.paramInt(6);
        goButton1.callback(callBackButton1);
        goButton1.dataPointType(DPT_Switch);

        ch2_on = knx.paramInt(10);
        ch2_off = knx.paramInt(14);
        goButton2.callback(callBackButton2);
        goButton2.dataPointType(DPT_Switch);

        ch3_on = knx.paramInt(18);
        ch3_off = knx.paramInt(22);
        goButton3.callback(callBackButton3);
        goButton3.dataPointType(DPT_Switch);

        chall_on =  knx.paramByte(26);
        chall_off = knx.paramByte(30);
        goButtonAll.callback(callBackButtonAll);
        goButtonAll.dataPointType(DPT_Switch);
        
        goProgMode.callback(callBackProgMode);
        goProgMode.dataPointType(DPT_Trigger);

        DPRINTLN("Finished");
    }

    knx.ledPin(5);
    knx.ledPinActiveOn(HIGH);
    knx.buttonPin(9);

    knx.start();
    led.set(2000, 2000);
}

void loop()
{
    knx.loop();
    led.loop();
    if (knx.configured() && !progMode)
    {
       button.loop();
    }
    else if (progMode)
    {
        prodModeLoop();
    }

}

void prodModeLoop(){ // run Only if progMode triggered ( at start or callback)

    const uint32_t timerProgMode = ( 15 * 60 * 1000 ) ; // 15min
    static uint32_t timerProgPrevMillis = 0;
    
    if (!knx.progMode())
    {
        knx.progMode(true);
        led.set(500, 500);
        timerProgPrevMillis = millis();
        DPRINTLN("progModeLoop Start");
    }
    else
    {
        if (millis() - timerProgPrevMillis > timerProgMode) {
            knx.progMode(false);
            goProgMode.value(false);
            progMode = 0;
            led.set(100, 100); // panic!
            DPRINTLN("progModeLoop Stop");
        }
    }
}
