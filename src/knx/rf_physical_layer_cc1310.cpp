#include "config.h"
#ifdef USE_RF

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rf_physical_layer.h"
#include "rf_data_link_layer.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/rf_data_entry.h)
#include <ti/drivers/rf/RF.h>
#include "smartrf_settings/smartrf_settings.h"

#include "cc1310_platform.h"
#include "Board.h"

#include "bits.h"
#include "platform.h"

#define RX_MAX_BUFFER_LENGTH 256
#define RF_TERMINATION_EVENT_MASK (RF_EventLastCmdDone | RF_EventLastFGCmdDone | RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled)

static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_CmdHandle rxCommandHandle;

static uint8_t rxBuffer[sizeof(rfc_dataEntryPartial_t) + RX_MAX_BUFFER_LENGTH] __attribute__((aligned(4)));
static rfc_dataEntryPartial_t* pDataEntry = (rfc_dataEntryPartial_t*)&rxBuffer;
static dataQueue_t dataQueue;

static uint8_t addrFilterTable[2] = {0x44, 0xFF}; // Do not modify the size without changing RF_cmdPropRxAdv.addrConf.addrSize!

static rfc_propRxOutput_t rxStatistics;

static uint8_t packetLength;
static uint8_t* packetDataPointer;
static int32_t packetStartTime;

static volatile int rf_done, rf_err, err;
int8_t len1, len2;

static void RxCallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventNDataWritten)
    {
        uint8_t *pData = &pDataEntry->rxData;
        if ((pData[1] != 0x44) || (pData[2] != 0xFF)) 
        {
           // cancel early because it does not seem to be KNX RF packet
           RF_cancelCmd(rfHandle, rxCommandHandle, 0 /*stop gracefully*/);
           return;
        }
        //uint8_t len = rxBuffer[sizeof(rfc_dataEntryPartial_t) + 0];
        uint8_t len = pDataEntry->rxData;
        struct rfc_CMD_PROP_SET_LEN_s RF_cmdPropSetLen =
        {
            .commandNo = CMD_PROP_SET_LEN,    // command identifier
            .rxLen   = (uint16_t)PACKET_SIZE(len)
        };

        len1=len; len2 = RF_cmdPropSetLen.rxLen;
        //RF_runImmediateCmd(rfHandle, (uint32_t*)&RF_cmdPropSetLen); // for length > 255
        RF_runDirectCmd(rfHandle, (uint32_t)&RF_cmdPropSetLen);
        packetStartTime = millis();
    }
    else if (e & RF_TERMINATION_EVENT_MASK) 
    {
        rf_done = true;
        rf_err = e & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled);
    }

    else /* unknown reason - should not occure */
    {
        pDataEntry->status = DATA_ENTRY_PENDING;
        err++;
    }
}


RfPhysicalLayer::RfPhysicalLayer(RfDataLinkLayer& rfDataLinkLayer, Platform& platform)
    : _rfDataLinkLayer(rfDataLinkLayer),
      _platform(platform)
{
}

void RfPhysicalLayer::setOutputPowerLevel(int8_t dBm)
{
    RF_TxPowerTable_Entry *rfPowerTable = NULL;
    RF_TxPowerTable_Value newValue;
    uint8_t rfPowerTableSize = 0;

    // Search the default PA power table for the desired power level
    newValue = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry *)PROP_RF_txPowerTable, dBm);
    if(newValue.rawValue != RF_TxPowerTable_INVALID_VALUE)
    {
        // Found a valid entry
        rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTable;
        rfPowerTableSize = PROP_RF_txPowerTableSize;
    }

    //if max power is requested then the CCFG_FORCE_VDDR_HH must be set in
    //the ccfg
#if (CCFG_FORCE_VDDR_HH != 0x1)
    if((newValue.paType == RF_TxPowerTable_DefaultPA) &&
       (dBm == rfPowerTable[rfPowerTableSize-2].power))
    {
        // The desired power level is set to the maximum supported under the
        // default PA settings, but the boost mode (CCFG_FORCE_VDDR_HH) is not
        // turned on
        return;
    }
#endif

    RF_Stat rfStatus = RF_setTxPower(rfHandle, newValue);
    if(rfStatus == RF_StatSuccess)
    {
        print("Successfully set TX output power to: ");
        println(newValue.rawValue);
    }
    else
    {
        print("Could not set TX output power to: ");
        println(newValue.rawValue);
    }
}


bool RfPhysicalLayer::InitChip()
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    pDataEntry->length = 255;
    pDataEntry->config.type = DATA_ENTRY_TYPE_PARTIAL; // --> DATA_ENTRY_TYPE_PARTIAL adds a 12 Byte Header
    pDataEntry-> config.irqIntv = 12; // KNX-RF first block consists of 12 bytes (one length byte, 0x44, 0xFF, one RFinfo byte, six Serial/DoA bytes, two CRC bytes)
    pDataEntry-> config.lenSz  = 0; // no length indicator at beginning of data entry
    pDataEntry->status = DATA_ENTRY_PENDING;
    pDataEntry->pNextEntry = (uint8_t*)pDataEntry;

    dataQueue.pCurrEntry = (uint8_t*)pDataEntry;
    dataQueue.pLastEntry = NULL;

    // Set buffer with address. We use the two fixed bytes 0x44 and 0xFF as our address to let the
    // packet engine do more filtering on itself
    RF_cmdPropRxAdv.pAddr = (uint8_t*)&addrFilterTable;
    // Set the Data Entity queue for received data
    RF_cmdPropRxAdv.pQueue = &dataQueue;             
    // Set the output buffer for RX packet statistics  
    RF_cmdPropRxAdv.pOutput = (uint8_t*)&rxStatistics;

    // Request access to the radio
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    return true;
}

void RfPhysicalLayer::stopChip()
{
    RF_cancelCmd(rfHandle, rxCommandHandle, 0 /* do not stop gracefully, instead hard abort RF */);
    RF_pendCmd(rfHandle, rxCommandHandle, RF_TERMINATION_EVENT_MASK);
    RF_yield(rfHandle);
    RF_close(rfHandle);
}

void RfPhysicalLayer::loop()
{
    switch (_loopState)
    {
    case TX_START:
        {
            println("TX_START...");
            _rfDataLinkLayer.loadNextTxFrame(&sendBuffer, &sendBufferLength);
            pktLen = PACKET_SIZE(sendBuffer[0]);
            if (pktLen != sendBufferLength)
            {
                printf("Error: SendBuffer[0]=%d, SendBufferLength=%d PACKET_SIZE=%d\n", sendBuffer[0], sendBufferLength, PACKET_SIZE(sendBuffer[0]));
            }
            // Calculate total number of bytes in the KNX RF packet from L-field
            // Check for valid length
            if ((pktLen == 0) || (pktLen > 290)) 
            {
                println("TX packet length error!");
                break;
            }

            if (pktLen > 255) 
            {
                println("Unhandled: TX packet > 255");
                break;
            }

             RF_cmdPropTx.pktLen = pktLen;
             RF_cmdPropTx.pPkt = sendBuffer;
             RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;
             RF_EventMask res = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_TERMINATION_EVENT_MASK);

            delete sendBuffer;

            if (res != RF_EventLastCmdDone) 
            {
                 printf("Unexpected result command %llu\n", res);
            }

            _loopState = RX_START;
        }
        break;

        case RX_START:
        {
             rf_done = rf_err = false;
             err = 0;

             rxCommandHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRxAdv, RF_PriorityNormal, &RxCallback, IRQ_RX_N_DATA_WRITTEN);
             if (rxCommandHandle == RF_ALLOC_ERROR) 
             {
                 println("Error: nRF_pendCmd() failed");
                 return;
             }
            _loopState = RX_ACTIVE;
        }
        break;

        case RX_ACTIVE:
        {
            if (!_rfDataLinkLayer.isTxQueueEmpty() && !syncStart)  
            {
                RF_cancelCmd(rfHandle, rxCommandHandle, RF_ABORT_GRACEFULLY);
                RF_pendCmd(rfHandle, rxCommandHandle, RF_TERMINATION_EVENT_MASK);
                pDataEntry->status = DATA_ENTRY_PENDING;
                _loopState = TX_START;
                break;
            }

            // Check if we have an incomplete packet reception
            if (!rf_done && syncStart && (millis() - packetStartTime > RX_PACKET_TIMEOUT)) 
            {
                println("RX packet timeout!");
                RF_cancelCmd(rfHandle, rxCommandHandle, RF_ABORT_GRACEFULLY);
                RF_pendCmd(rfHandle, rxCommandHandle, RF_TERMINATION_EVENT_MASK);
                pDataEntry->status = DATA_ENTRY_PENDING;
                _loopState = RX_START;
                break;
            }
            else if (rf_done) 
            {
                RF_EventMask res = RF_pendCmd(rfHandle, rxCommandHandle, RF_TERMINATION_EVENT_MASK);
                if (res == RF_EventCmdCancelled || res == RF_EventCmdStopped || res == RF_EventCmdAborted) 
                {
                    println("RF terminated because of RF_flushCmd() or RF_cancelCmd()");
                }
                else if (res != RF_EventLastCmdDone) 
                {
                    //printf("Unexpected Rx result command %llu\n", res);
                    print("Unexpected Rx result command: ");
                    println(res, HEX);
                }
                else if (rf_err) 
                {
                    println("Rx is no KNX frame");
                } 
                else 
                {
                    //printf("len1=%d, len1=%d, frags=%d, err=%d\n", len1, len2, frags, err);
                    print("nRxOk = ");println(rxStatistics.nRxOk);           //!<        Number of packets that have been received with payload, CRC OK and not ignored
                    print("nRxNok = ");println(rxStatistics.nRxNok);         //!<        Number of packets that have been received with CRC error
                    print("nRxIgnored = ");println(rxStatistics.nRxIgnored); //!<        Number of packets that have been received with CRC OK and ignored due to address mismatch
                    print("nRxStopped = ");println(rxStatistics.nRxStopped); //!<        Number of packets not received due to illegal length or address mismatch with pktConf.filterOp = 1
                    print("nRxBufFull = ");println(rxStatistics.nRxBufFull); //!<        Number of packets that have been received and discarded due to lack of buffer space
                    print("lastRssi = ");println(rxStatistics.lastRssi);     //!<        RSSI of last received packet

                    // add CRC sizes for received blocks, but do not add the length of the L-field (1 byte) itself
                    packetLength = PACKET_SIZE(pDataEntry->rxData); 
                    packetDataPointer = (uint8_t *) &pDataEntry->rxData;

                    if (packetLength+1 != pDataEntry->nextIndex) 
                    {
                        //printf("Size mismatch: %d %d\n", packetLength, *(uint8_t *)(&(currentDataEntry->data)+2));
                        //printf("Size mismatch: %d %d\n", packetLength, pDataEntry->nextIndex);
                        println("Size mismatch");
                    }

                    printHex("RX: ", packetDataPointer, packetLength);
                    _rfDataLinkLayer.frameBytesReceived(packetDataPointer, packetLength);
                    pDataEntry->status = DATA_ENTRY_PENDING;
                }
                _loopState = RX_START;
            }
        }
        break;
    }
}

#endif
