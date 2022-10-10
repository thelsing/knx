#include <Arduino.h>
#include <knx.h>

#define NUMIOS 18

#define goInputOnOff(i) knx.getGroupObject(i + 1)
#define goOutputOnOff(i) knx.getGroupObject(i + 1 + NUMIOS)
#define goOutputScaling(i) knx.getGroupObject(i + 1 + NUMIOS * 2)
#define goOutputOnOffStatus(i) knx.getGroupObject(i + 1 + NUMIOS * 3)
#define goOutputScalingStatus(i) knx.getGroupObject(i + 1 + NUMIOS * 4)

typedef enum {
    CONFIG_NONE = 0,
    CONFIG_IN = 1,
    CONFIG_IN_TOGGLE = 2,
    CONFIG_IN_TOGGLE_ON = 3,
    CONFIG_OUT = 4,
    CONFIG_OUT_ON = 5,
    CONFIG_OUT_PWM = 6,
    CONFIG_OUT_PWM_ON = 7,
} config_t;

inline void isrIn(uint8_t pin, uint8_t toggle);

// workaround for the weird Arduino interrupt handling
#define ISR_IN(NUM, TOGGLE) void isrIn##NUM##_##TOGGLE(){isrIn(NUM, TOGGLE);}
ISR_IN(0,0);
ISR_IN(1,0);
ISR_IN(2,0);
ISR_IN(3,0);
ISR_IN(4,0);
ISR_IN(5,0);
ISR_IN(6,0);
ISR_IN(7,0);
ISR_IN(8,0);
ISR_IN(9,0);
ISR_IN(10,0);
ISR_IN(11,0);
ISR_IN(12,0);
ISR_IN(13,0);
ISR_IN(14,0);
ISR_IN(15,0);
ISR_IN(16,0);
ISR_IN(17,0);
ISR_IN(0,1);
ISR_IN(1,1);
ISR_IN(2,1);
ISR_IN(3,1);
ISR_IN(4,1);
ISR_IN(5,1);
ISR_IN(6,1);
ISR_IN(7,1);
ISR_IN(8,1);
ISR_IN(9,1);
ISR_IN(10,1);
ISR_IN(11,1);
ISR_IN(12,1);
ISR_IN(13,1);
ISR_IN(14,1);
ISR_IN(15,1);
ISR_IN(16,1);
ISR_IN(17,1);

const uint32_t pinTbl[NUMIOS] = {PA15, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PC13, PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0};
void (*inCbTbl[NUMIOS * 2])(void) = {isrIn0_0, isrIn1_0, isrIn2_0, isrIn3_0, isrIn4_0, isrIn5_0, isrIn6_0, isrIn7_0, isrIn8_0, isrIn9_0, isrIn10_0, isrIn11_0, isrIn12_0, isrIn13_0, isrIn14_0, isrIn15_0, isrIn16_0, isrIn17_0, isrIn0_1, isrIn1_1, isrIn2_1, isrIn3_1, isrIn4_1, isrIn5_1, isrIn6_1, isrIn7_1, isrIn8_1, isrIn9_1, isrIn10_1, isrIn11_1, isrIn12_1, isrIn13_1, isrIn14_1, isrIn15_1, isrIn16_1, isrIn17_1};
volatile uint16_t val[NUMIOS];
uint32_t lastEvent[NUMIOS];

// callback for OnOff events from KNX
void outOnOff(GroupObject& go)
{
    uint8_t pin = go.asap() - 1 - NUMIOS;
    val[pin] = go.value();
    val[pin] &= 1;
    digitalWrite(pinTbl[pin], val[pin]);
    goOutputOnOffStatus(pin).value(val[pin]);
}

// callback for OnOff events from KNX on PWM pins
void outOnOffPWM(GroupObject& go)
{
    uint8_t pin = go.asap() - 1 - NUMIOS;
    uint8_t tmp;
    if(go.value()){
        val[pin] |= 0x100;
        analogWrite(pinTbl[pin], val[pin] & 0xff);
    }else{
        val[pin] &= 0xff;
        analogWrite(pinTbl[pin], 0);
    }
    tmp = val[pin] >> 8;
    goOutputOnOffStatus(pin).value(tmp);
}

// callback for 0-100% events from KNX
void outScaling(GroupObject& go)
{
    uint8_t pin = go.asap() - NUMIOS * 2 - 1;
    uint8_t tmp;
    if(val[pin] > 0xFF){
        tmp = *go.valueRef();
        val[pin] = tmp | 1 << 8;
        analogWrite(pinTbl[pin], val[pin] & 0xff);
    }else{
        tmp = *go.valueRef();
        val[pin] = tmp;
    }
    *goOutputScalingStatus(pin).valueRef() = tmp;
    goOutputScalingStatus(pin).objectWritten();
}

// callback for input interrupts
inline void isrIn(uint8_t pin, uint8_t toggle)
{
    uint32_t diff = millis() - lastEvent[pin];
    if (diff >= 50 && diff <= 500){
        if(toggle){
            val[pin]++;
        }else{
            val[pin] = digitalRead(pinTbl[pin]);
        }
        val[pin] &= 1;
        goInputOnOff(pin).value(val[pin]);
    }

    lastEvent[pin] = millis();
}

void setup()
{
    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // print values of parameters if device is already configured
    if (knx.configured())
    {
        uint8_t progIn = knx.paramByte(0); // programming input
        uint8_t progLed = knx.paramByte(1); // programming LED

        for(uint8_t i = 0; i < NUMIOS; i++){
            config_t config = (config_t) knx.paramByte(i + 2);
            // loop through all the pins and configure them correctly
            switch(config){
                case CONFIG_IN:
                    pinMode(pinTbl[i], INPUT_PULLUP);
                    if(progIn = i + 1){
                        knx.buttonPin(pinTbl[i]);
                    }else{
                        goInputOnOff(i).dataPointType(DPT_Switch);
                        #if (ARDUINO_API_VERSION >= 10200)
                            attachInterrupt(digitalPinToInterrupt(pinTbl[i]), inCbTbl[i], (PinStatus)CHANGE);
                        #else
                            attachInterrupt(digitalPinToInterrupt(pinTbl[i]), inCbTbl[i], CHANGE);
                        #endif
                    }
                    break;
                case CONFIG_IN_TOGGLE:
                case CONFIG_IN_TOGGLE_ON:
                    goInputOnOff(i).dataPointType(DPT_Switch);
                    val[i] = config == CONFIG_IN_TOGGLE_ON;
                    pinMode(pinTbl[i], INPUT_PULLUP);
                    #if (ARDUINO_API_VERSION >= 10200)
                        attachInterrupt(digitalPinToInterrupt(pinTbl[i]), inCbTbl[i + NUMIOS], (PinStatus)CHANGE);
                    #else
                        attachInterrupt(digitalPinToInterrupt(pinTbl[i]), inCbTbl[i + NUMIOS], CHANGE);
                    #endif
                    break;
                case CONFIG_OUT:
                case CONFIG_OUT_ON:
                    pinMode(pinTbl[i], OUTPUT);
                    val[i] = config == CONFIG_OUT_ON;
                    digitalWrite(pinTbl[i], val[i]);
                    if(progLed = i + 1){
                        knx.ledPin(pinTbl[i]);
                    }else{
                        goOutputOnOff(i).dataPointType(DPT_Switch);
                        goOutputOnOff(i).callback(outOnOff);
                        goOutputOnOffStatus(i).dataPointType(DPT_Switch);
                    }
                    break;
                case CONFIG_OUT_PWM:
                case CONFIG_OUT_PWM_ON:
                    pinMode(pinTbl[i], OUTPUT);
                    val[i] = config == CONFIG_OUT_PWM_ON ? 0 : 0x1ff;
                    analogWrite(pinTbl[i], val[i] & 0xff);
                    goOutputOnOff(i).dataPointType(DPT_Switch);
                    goOutputOnOff(i).callback(outOnOffPWM);
                    goOutputOnOffStatus(i).dataPointType(DPT_Switch);
                    goOutputScaling(i).dataPointType(DPT_Scaling);
                    goOutputScaling(i).callback(outScaling);
                    goOutputScalingStatus(i).dataPointType(DPT_Scaling);
                    break;
            }
        }

    }

    // start the framework.
    knx.start();
}

void loop()
{
    // don't delay here too much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if (!knx.configured())
        return;
}
