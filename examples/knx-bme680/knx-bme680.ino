#include <bsec.h>
#include <knx_esp.h>

// declare array of all groupobjects with their sizes in byte
GroupObject groupObjects[]
{
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(2),
    GroupObject(1),
    GroupObject(2),
    GroupObject(2),
    GroupObject(1),
    GroupObject(2)
}
;

// create named references for easy access to group objects
GroupObject& goRawTemperature = groupObjects[0];
GroupObject& goPressure = groupObjects[1];
GroupObject& goRawHumidity = groupObjects[2];
GroupObject& goGasResistance = groupObjects[3];
GroupObject& goIaqEstimate = groupObjects[4];
GroupObject& goIaqAccurace = groupObjects[5];
GroupObject& goTemperature = groupObjects[6];
GroupObject& goHumidity = groupObjects[7];
GroupObject& goTriggerSample = groupObjects[8];
GroupObject& goCo2Ppm = groupObjects[9];

#define STATE_SAVE_PERIOD  UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);
uint8_t* saveBme680State(uint8_t* buffer);
uint8_t* loadBme680State(uint8_t* buffer);
void triggerCallback(GroupObject& go);
void updateState();

///const uint8_t bsec_config_iaq[304] = 
//     { 0, 6, 4, 1, 61, 0, 0, 0, 0, 0, 0, 0, 24, 1, 0, 0, 40, 0, 1, 0, 137, 65, 0, 63, 0, 0, 64, 63, 205, 204, 76, 62, 0, 0, 225, 68, 0, 192, 168, 71, 0, 0, 0, 0, 0, 80, 10, 90, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 2, 0, 0, 244, 1, 225, 0, 25, 10, 144, 1, 0, 0, 112, 65, 0, 0, 0, 63, 16, 0, 3, 0, 10, 215, 163, 60, 10, 215, 35, 59, 10, 215, 35, 59, 9, 0, 5, 0, 0, 0, 0, 0, 1, 51, 0, 9, 0, 10, 215, 163, 59, 205, 204, 204, 61, 225, 122, 148, 62, 41, 92, 15, 61, 0, 0, 0, 63, 0, 0, 0, 63, 154, 153, 89, 63, 154, 153, 25, 62, 1, 1, 0, 0, 128, 63, 6, 236, 81, 184, 61, 51, 51, 131, 64, 12, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 0, 254, 0, 2, 1, 5, 48, 117, 100, 0, 44, 1, 151, 7, 132, 3, 197, 0, 144, 1, 64, 1, 64, 1, 48, 117, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 48, 117, 100, 0, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 44, 1, 0, 0, 0, 0, 98, 149, 0, 0 };
const uint8_t bsec_config_iaq[304] = 
     { 0, 6, 4, 1, 61, 0, 0, 0, 0, 0, 0, 0, 24, 1, 0, 0, 40, 0, 1, 0, 137, 65, 0, 63, 0, 0, 64, 63, 205, 204, 76, 62, 0, 0, 225, 68, 0, 168, 19, 73, 0, 0, 0, 0, 0, 80, 10, 90, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 2, 0, 0, 244, 1, 225, 0, 25, 10, 144, 1, 0, 0, 112, 65, 0, 0, 0, 63, 16, 0, 3, 0, 10, 215, 163, 60, 10, 215, 35, 59, 10, 215, 35, 59, 9, 0, 5, 0, 0, 0, 0, 0, 1, 51, 0, 9, 0, 10, 215, 163, 59, 205, 204, 204, 61, 225, 122, 148, 62, 41, 92, 15, 61, 0, 0, 0, 63, 0, 0, 0, 63, 154, 153, 89, 63, 154, 153, 25, 62, 1, 1, 0, 0, 128, 63, 6, 236, 81, 184, 61, 51, 51, 131, 64, 12, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 0, 254, 0, 2, 1, 5, 48, 117, 100, 0, 44, 1, 151, 7, 132, 3, 197, 0, 144, 1, 64, 1, 64, 1, 48, 117, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 48, 117, 100, 0, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 44, 1, 0, 0, 0, 0, 253, 222, 0, 0 };


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
    // register group objects
    knx.registerGroupObjects(groupObjects, 10);

    // read adress table, association table, groupobject table and parameters from eeprom
    knx.readMemory();

    // register callback for reset GO
    goTriggerSample.updateHandler = triggerCallback;
    

    iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
    checkIaqSensorStatus();

    iaqSensor.setConfig(bsec_config_iaq);
    checkIaqSensorStatus();

    bsec_virtual_sensor_t sensorList[7] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ_ESTIMATE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };

    knx.setSaveCallback(saveBme680State);
    knx.setRestoreCallback(loadBme680State);
    
    if (knx.configured())
    {
        cyclSend = knx.paramInt(0);
        Serial.print("Zykl. send:");
        Serial.println(cyclSend);
    }
    
    // start the framework. Will get wifi first.
    knx.start();
    
    iaqSensor.updateSubscription(sensorList, 7, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();
    String output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%]";
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
        output += ", " + String(iaqSensor.iaqEstimate);
        output += ", " + String(iaqSensor.iaqAccuracy);
        output += ", " + String(iaqSensor.temperature);
        output += ", " + String(iaqSensor.humidity);
        Serial.println(output);
        updateState();
        
        if (sendCounter++ == cyclSend || trigger)
        {
            sendCounter = 0;
            trigger = false;
        
            goRawTemperature.objectWrite(iaqSensor.rawTemperature);
            goPressure.objectWrite(iaqSensor.pressure);
            goRawHumidity.objectWrite(iaqSensor.rawHumidity);
            goGasResistance.objectWrite(iaqSensor.gasResistance);
            goIaqEstimate.objectWrite(iaqSensor.iaqEstimate);
            goIaqAccurace.objectWrite(iaqSensor.iaqAccuracy);
            goTemperature.objectWrite(iaqSensor.temperature);
            goHumidity.objectWrite(iaqSensor.humidity);
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
    Serial.println(go.objectReadBool());
    if (!go.objectReadBool())
        return;

    trigger = true;
    /* We call bsec_update_subscription() in order to instruct BSEC to perform an extra measurement at the next
     possible time slot
     */
    Serial.println("Triggering ULP plus.");
    bsec_virtual_sensor_t sensorList[1] = {
        BSEC_OUTPUT_IAQ_ESTIMATE,
    };

    iaqSensor.updateSubscription(sensorList, 1, BSEC_SAMPLE_RATE_ULP_MEASUREMENT_ON_DEMAND);
    checkIaqSensorStatus();
}