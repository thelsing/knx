// used version: BSEC_1.4.7.3_Generic_Release_20190410
#include <bsec.h>
#include <knx.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <WiFiManager.h>
#endif


// create named references for easy access to group objects
#define goRawTemperature knx.getGroupObject(1)
#define goPressure knx.getGroupObject(2)
#define goRawHumidity knx.getGroupObject(3)
#define goGasResistance knx.getGroupObject(4)
#define goIaqEstimate knx.getGroupObject(5)
#define goIaqAccurace knx.getGroupObject(6)
#define goTemperature knx.getGroupObject(7)
#define goHumidity knx.getGroupObject(8)
#define goTriggerSample knx.getGroupObject(9)
#define goCo2Ppm knx.getGroupObject(10)

#define STATE_SAVE_PERIOD  UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);
uint8_t* saveBme680State(uint8_t* buffer);
uint8_t* loadBme680State(uint8_t* buffer);
void triggerCallback(GroupObject& go);
void updateState();

// from generic_33v_3s_28d
const uint8_t bsec_config_iaq[454] =
    {3, 7, 4, 1, 61, 0, 0, 0, 0, 0, 0, 0, 174, 1, 0, 0, 48, 0, 1, 0, 0, 168, 19, 73, 64, 49, 119, 76, 0, 0, 225, 68, 137, 65, 0, 63, 205, 204, 204, 62, 0, 0, 64, 63, 205, 204, 204, 62, 0, 0, 0, 0, 0, 80, 5, 95, 0, 0, 0, 0, 0, 0, 0, 0, 28, 0, 2, 0, 0, 244, 1, 225, 0, 25, 0, 0, 128, 64, 0, 0, 32, 65, 144, 1, 0, 0, 112, 65, 0, 0, 0, 63, 16, 0, 3, 0, 10, 215, 163, 60, 10, 215, 35, 59, 10, 215, 35, 59, 9, 0, 5, 0, 0, 0, 0, 0, 1, 88, 0, 9, 0, 7, 240, 150, 61, 0, 0, 0, 0, 0, 0, 0, 0, 28, 124, 225, 61, 52, 128, 215, 63, 0, 0, 160, 64, 0, 0, 0, 0, 0, 0, 0, 0, 205, 204, 12, 62, 103, 213, 39, 62, 230, 63, 76, 192, 0, 0, 0, 0, 0, 0, 0, 0, 145, 237, 60, 191, 251, 58, 64, 63, 177, 80, 131, 64, 0, 0, 0, 0, 0, 0, 0, 0, 93, 254, 227, 62, 54, 60, 133, 191, 0, 0, 64, 64, 12, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 229, 0, 254, 0, 2, 1, 5, 48, 117, 100, 0, 44, 1, 112, 23, 151, 7, 132, 3, 197, 0, 92, 4, 144, 1, 64, 1, 64, 1, 144, 1, 48, 117, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 100, 0, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 255, 255, 255, 255, 255, 255, 255, 255, 220, 5, 220, 5, 220, 5, 255, 255, 255, 255, 255, 255, 220, 5, 220, 5, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 44, 1, 0, 0, 0, 0, 96, 159, 0, 0};

// Create an object of the class Bsec
Bsec iaqSensor;
uint16_t stateUpdateCounter = 0;
uint8_t sendCounter = 0;
uint32_t cyclSend = 0;
bool trigger = false;

// Entry point for the example
void setup(void)
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("start");

    #ifdef ARDUINO_ARCH_ESP8266
    WiFiManager wifiManager;    
    wifiManager.autoConnect("knx-bme680");
    #endif

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // register callback for reset GO
    if(knx.configured())
        goTriggerSample.callback(triggerCallback);
    

    iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    checkIaqSensorStatus();

    iaqSensor.setConfig(bsec_config_iaq);
    checkIaqSensorStatus();

    bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
        BSEC_OUTPUT_CO2_EQUIVALENT
    };

    knx.setSaveCallback(saveBme680State);
    knx.setRestoreCallback(loadBme680State);
    
    if (knx.configured())
    {
        cyclSend = knx.paramInt(0);
        Serial.print("Zykl. send:");
        Serial.println(cyclSend);
        goRawTemperature.dataPointType(Dpt(9, 0));
        goPressure.dataPointType(Dpt(9, 0));
        goRawHumidity.dataPointType(Dpt(9, 0));
        goGasResistance.dataPointType(Dpt(9, 0));
        goIaqEstimate.dataPointType(Dpt(9, 0));
        goIaqAccurace.dataPointType(Dpt(9, 0));
        goTemperature.dataPointType(Dpt(9, 0));
        goHumidity.dataPointType(Dpt(9, 0));
        goCo2Ppm.dataPointType(Dpt(9, 0));
        goTriggerSample.dataPointType(Dpt(1, 0));
    }
    
    // start the framework. Will get wifi first.
    knx.start();
    
    iaqSensor.updateSubscription(sensorList, 7, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();
    String output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], CO2";
    Serial.println(output);
}

// Function that is looped forever
void loop(void)
{
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();

    // only run the application code if the device was configured with ETS
    if(!knx.configured())
        return;
    
    if (iaqSensor.run()) 
    {
        String output = String(millis());
        
        output += ", " + String(iaqSensor.rawTemperature);
        output += ", " + String(iaqSensor.pressure);
        output += ", " + String(iaqSensor.rawHumidity);
        output += ", " + String(iaqSensor.gasResistance);
        output += ", " + String(iaqSensor.iaq);
        output += ", " + String(iaqSensor.iaqAccuracy);
        output += ", " + String(iaqSensor.temperature);
        output += ", " + String(iaqSensor.humidity);
        output += ", " + String(iaqSensor.co2Equivalent);
        output += ", " + String(iaqSensor.co2Accuracy);

        Serial.println(output);
        updateState();
        
        if (sendCounter++ == cyclSend || trigger)
        {
            sendCounter = 0;
            trigger = false;
        
            goRawTemperature.value(iaqSensor.rawTemperature);
            goPressure.value(iaqSensor.pressure);
            goRawHumidity.value(iaqSensor.rawHumidity);
            goGasResistance.value(iaqSensor.gasResistance);
            goIaqEstimate.value(iaqSensor.iaq);
            goIaqAccurace.value(iaqSensor.iaqAccuracy);
            goTemperature.value(iaqSensor.temperature);
            goHumidity.value(iaqSensor.humidity);
            goCo2Ppm.value(iaqSensor.co2Equivalent);
        }
    }
    else {
        checkIaqSensorStatus();
    }
}

// Helper function definitions
void checkIaqSensorStatus(void)
{
    if (iaqSensor.status != BSEC_OK) {
        if (iaqSensor.status < BSEC_OK) {
            String output = "BSEC error code : " + String(iaqSensor.status);
            Serial.println(output);
            for (;;)
                errLeds(); /* Halt in case of failure */
        }
        else {
            String output = "BSEC warning code : " + String(iaqSensor.status);
            Serial.println(output);
        }
    }

    if (iaqSensor.bme680Status != BME680_OK) {
        if (iaqSensor.bme680Status < BME680_OK) {
            String output = "BME680 error code : " + String(iaqSensor.bme680Status);
            Serial.println(output);
            for (;;)
                errLeds(); /* Halt in case of failure */
        }
        else {
            String output = "BME680 warning code : " + String(iaqSensor.bme680Status);
            Serial.println(output);
        }
    }
}

void errLeds(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

uint8_t* loadBme680State(uint8_t* buffer)
{
    // Existing state in EEPROM
    Serial.println("Reading state from EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        Serial.println(buffer[i], HEX);
    }

    iaqSensor.setState(buffer);
    checkIaqSensorStatus();
    return buffer + BSEC_MAX_STATE_BLOB_SIZE;
}

uint8_t* saveBme680State(uint8_t* buffer)
{
    iaqSensor.getState(buffer);
    checkIaqSensorStatus();

    Serial.println("Writing state to EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        Serial.println(buffer[i], HEX);
    }
    return buffer + BSEC_MAX_STATE_BLOB_SIZE;
}

void updateState(void)
{
    bool update = false;
    if (stateUpdateCounter == 0) {
        /* First state update when IAQ accuracy is >= 1 */
        if (iaqSensor.iaqAccuracy >= 3) {
            update = true;
            stateUpdateCounter++;
        }
    }
    else {
        /* Update every STATE_SAVE_PERIOD minutes */
        if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
            update = true;
            stateUpdateCounter++;
        }
    }

    if (update) {
        knx.writeMemory();
    }
}

// callback from trigger-GO
void triggerCallback(GroupObject& go)
{
    Serial.println("trigger");
    Serial.println((bool)go.value());
    if (!go.value())
        return;

    trigger = true;
    /* We call bsec_update_subscription() in order to instruct BSEC to perform an extra measurement at the next
     possible time slot
     */
    Serial.println("Triggering ULP plus.");
    bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_IAQ, BSEC_OUTPUT_CO2_EQUIVALENT
    };

    iaqSensor.updateSubscription(sensorList, 1, BSEC_SAMPLE_RATE_ULP_MEASUREMENT_ON_DEMAND);
    checkIaqSensorStatus();
}