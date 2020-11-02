/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Copyright(c) 2020 - Matthias Meier */


#include "config.h"
#ifdef USE_RF

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rf_physical_layer.h"
#include "rf_data_link_layer.h"

#include "smartrf_settings/smartrf_settings.h"
#include <ti/drivers/rf/RF.h>
#include "RFQueue.h"

#include "cc1310_platform.h"
#include "Board.h"

#include "bits.h"
#include "platform.h"

static RF_Object rfObject;
RF_Handle rfHandle;
RF_CmdHandle rxCommandHandle;
uint8_t lastRssi;

#define DATA_ENTRY_HEADER_SIZE  12 /* Constant header size of a Generic Data Entry - 12 for DATA_ENTRY_TYPE_PARTIAL, 8 for DATA_ENTRY_TYPE_GEN */
#define MAX_LENGTH              255
#define LENGTH_POSITION         0
#define APPENDED_BYTES          10

static uint8_t rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + MAX_LENGTH + LENGTH_POSITION + APPENDED_BYTES] __attribute__((aligned(4)));
rfc_dataEntryPartial_t* partialReadEntry = (rfc_dataEntryPartial_t*)&rxDataEntryBuffer;

static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

volatile static int frags;
volatile static int rf_done, rf_err, err;
int8_t len1, len2;

rfc_propRxOutput_t rxStatistics;
static volatile uint8_t lengthWritten = false;
static volatile uint8_t rxDone = false;
int32_t packetStartTime;

#define PACKET_SIZE(lField) ((((lField - 10 /*size of first pkt*/))/16 + 2 /*CRC in first pkt */) * 2 /*to bytes*/ +lField + 1 /*size of len byte*/)

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    frags++;
    if (e & RF_EventNDataWritten && frags==1)
    {
        //uint16_t block1Crc = rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + 10] << 8 | rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + 11];
        if ((rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + 1] != 0x44) || (rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + 2] != 0xFF) /*|| (crc16(&rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE], 0, 10) != block1Crc)*/) {
           // cancel because not a KNX package ID
           RF_cancelCmd(rfHandle, rxCommandHandle, 0 /*stop gracefully*/);
           return;
        }
        uint8_t len = rxDataEntryBuffer[DATA_ENTRY_HEADER_SIZE + 0];
        struct rfc_CMD_PROP_SET_LEN_s RF_cmdPropSetLen =
        {
            .commandNo = CMD_PROP_SET_LEN,    // command identifier
            .rxLen   = (uint16_t)PACKET_SIZE(len)
        };

        len1=len; len2 = RF_cmdPropSetLen.rxLen;
        //RF_cmdPropSetLen.rxLen = 40;
        RF_runDirectCmd(rfHandle, (uint32_t)&RF_cmdPropSetLen);
        packetStartTime = millis();
    }
    else if (e & (RF_EventLastCmdDone | RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled)) {
        rf_done = true;
        rf_err = e & (RF_EventCmdStopped | RF_EventCmdAborted | RF_EventCmdCancelled);
    }

    else /* unknown reason - should not occure */
    {
        partialReadEntry->status = DATA_ENTRY_PENDING;
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
/*
 * TODO: Complete if needed
 * Refs:
 * https://e2e.ti.com/support/wireless-connectivity/sub-1-ghz/f/156/t/540423?How-to-dynamically-change-tx-power-from-10dBm-to-14dBm-when-running-CC1310-application-
 * https://e2e.ti.com/support/wireless-connectivity/sub-1-ghz/f/156/t/561302?Increase-TX-power-of-cc1310-stepwise
 * Power Table from Conticki:
 const prop_mode_tx_power_config_t prop_mode_tx_power_779_930[] = {
   {  14, 0xa73f },   // needs CCFG_FORCE_VDDR_HH=1
   {  13, 0xa63f },   // 12.5dB
   {  12, 0xb818 },
   {  11, 0x50da },
   {  10, 0x38d3 },
   {   9, 0x2ccd },
   {   8, 0x24cb },
   {   7, 0x20c9 },
   {   6, 0x1cc7 },
   {   5, 0x18c6 },
   {   4, 0x18c5 },
   {   3, 0x14c4 },
   {   2, 0x1042 },
   {   1, 0x10c3 },
   {   0, 0x0041 },
   { -10, 0x08c0 },
   {-128, 0xFFFF },
 };
 * code to change power to eg. 14dBm :
 * RF_yield(rfHandle);
 * RF_cmdPropRadioDivSetup.txPower = 0xA73F;
 * RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
 */
}


bool RfPhysicalLayer::InitChip()
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    if( RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            1,
                            MAX_LENGTH + APPENDED_BYTES))
    {
        println("Failed to allocate space for all data entries");
        while(1);
    }

    partialReadEntry->length = 255;
    partialReadEntry->config.type = DATA_ENTRY_TYPE_PARTIAL;    // --> DATA_ENTRY_TYPE_PARTIAL adds a 12 Byte Header
    partialReadEntry-> config.irqIntv = 12;
    partialReadEntry-> config.lenSz  = 0;   // len field not handeld by rf core because it does not include CRC bytes
    partialReadEntry->status = DATA_ENTRY_PENDING;
    partialReadEntry->pNextEntry = (uint8_t*)partialReadEntry;

    dataQueue.pCurrEntry = (uint8_t*)partialReadEntry;
    dataQueue.pLastEntry = NULL;

    /* 4.1. Modify CMD_PROP_RX command for application needs */
    RF_cmdPropRx.pQueue = &dataQueue;               /* Set the Data Entity queue for received data */
    RF_cmdPropRx.maxPktLen = 0;                     /* Unlimited length */
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 0x0;     /* Auto-flush packets with invalid CRC */
    RF_cmdPropRx.pktConf.bRepeatNok = 0x0;          /* Exit RX after a packet is recived */
    RF_cmdPropRx.pktConf.bRepeatOk = 0x0;           /* Exit RX after a packet is recived */
    RF_cmdPropRx.pOutput = (uint8_t*)&rxStatistics;

    /* Request access to the radio */
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    return true;
}

void RfPhysicalLayer::stopChip()
{
    RF_cancelCmd(rfHandle, rxCommandHandle, 0 /*stop RF abort */);
    RF_pendCmd(rfHandle, rxCommandHandle, 0);
    RF_yield(rfHandle);
}


uint16_t RfPhysicalLayer::packetSize (uint8_t lField)
{
  uint16_t nrBytes;
  uint8_t  nrBlocks;
  
  // The 2 first blocks contains 25 bytes when excluding CRC and the L-field
  // The other blocks contains 16 bytes when excluding the CRC-fields
  // Less than 26 (15 + 10) 
  if ( lField < 26 ) 
    nrBlocks = 2;
  else 
    nrBlocks = (((lField - 26) / 16) + 3);
  
  // Add all extra fields, excluding the CRC fields
  nrBytes = lField + 1;

  // Add the CRC fields, each block has 2 CRC bytes
  nrBytes += (2 * nrBlocks);
  return nrBytes;
}

void RfPhysicalLayer::loop()
{
    static uint8_t lastRxOk;

    switch (_loopState)
    {
    case TX_START:
        {
            println("TX_START...");
            _rfDataLinkLayer.loadNextTxFrame(&sendBuffer, &sendBufferLength);
            pktLen = PACKET_SIZE(sendBuffer[0]);
            //pktLen = packetSize(sendBuffer[0]);
            if (PACKET_SIZE(sendBuffer[0]) != packetSize(sendBuffer[0]) || PACKET_SIZE(sendBuffer[0]) != sendBufferLength)
            {
                printf("Error: SendBuffer[0]=%d, SendBufferLength=%d PACKET_SIZE=%d, packetSize=%d\n", sendBuffer[0], sendBufferLength, PACKET_SIZE(sendBuffer[0]), packetSize(sendBuffer[0]));
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
             RF_EventMask res = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);

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
             //print("RX_START...\n");
             frags = 0;
             rf_done = rf_err = false;
             err = 0;
             lastRxOk = rxStatistics.nRxOk;
             //RF_EventMask res = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, IRQ_RX_N_DATA_WRITTEN);

             rxCommandHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, IRQ_RX_N_DATA_WRITTEN);
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
                RF_cancelCmd(rfHandle, rxCommandHandle, 1 /*stop gracefully*/);
                RF_pendCmd(rfHandle, rxCommandHandle, 0);
                RFQueue_nextEntry();
                _loopState = TX_START;
                break;
            }

            // Check if we have an incomplete packet reception
            if (!rf_done && syncStart && (millis() - packetStartTime > RX_PACKET_TIMEOUT)) 
            {
                println("RX packet timeout!");
                RF_cancelCmd(rfHandle, rxCommandHandle, 1 /*stop gracefully*/);
                RF_pendCmd(rfHandle, rxCommandHandle, 0);
                RFQueue_nextEntry();
                _loopState = RX_START;
                break;
            }
            else if (rf_done) 
            {
                RF_EventMask res = RF_pendCmd(rfHandle, rxCommandHandle, 0);
/*                
                if (rxStatistics.nRxOk == lastRxOk) 
                {
                    println("Rx empty or invalid");
                }
                else if ( rxStatistics.lastRssi < -120) 
                {
                    println("\nIgnoring Rx with rssi < -120dB");
                }
*/                
                if (res == RF_EventCmdCancelled || res == RF_EventCmdStopped || res == RF_EventCmdAborted) 
                {
                    println("RF terminated because of  RF_flushCmd() or RF_cancelCmd()");
                }
                else if (res != RF_EventLastCmdDone) 
                {
                    printf("Unexpected Rx result command %llu\n", res);
                }
                else if (rf_err) 
                {
                    print("Rx is no KNX frame\n\n");
                } 
                else 
                {
                   printf("len1=%d, len1=%d, frags=%d, err=%d\n", len1, len2, frags, err);
                   printf("nRxOk = %d ", rxStatistics.nRxOk);                      //!<        Number of packets that have been received with payload, CRC OK and not ignored
                   printf("nRxNok = %d ", rxStatistics.nRxNok);                     //!<        Number of packets that have been received with CRC error
                   printf("nRxIgnored = %d ", rxStatistics.nRxIgnored);                  //!<        Number of packets that have been received with CRC OK and ignored due to address mismatch
                   printf("nRxStopped = %d ", rxStatistics.nRxStopped);                  //!<        Number of packets not received due to illegal length or address mismatch with pktConf.filterOp = 1
                   printf("nRxBufFull = %d ", rxStatistics.nRxBufFull);                  //!<        Number of packets that have been received and discarded due to lack of buffer space
                   printf("lastRssi = %d\n", rxStatistics.lastRssi);                     //!<        RSSI of last received packet

                   currentDataEntry = RFQueue_getDataEntry();
                   /* Handle the packet data, located at &currentDataEntry->data:
                    * For unknown reason, there is a header of 4 bytes whereas nr bytes rececived +1 is at offset [2]*/
                   packetLength      = *(uint8_t *)(&(currentDataEntry->data)+4);

                   if (PACKET_SIZE(packetLength) != PACKET_SIZE(packetLength))
                     printf("Error RX: packetLength=%d PACKET_SIZE=%d, packetSize=%d\n", packetLength, PACKET_SIZE(packetLength), packetSize(packetLength));

                   packetLength = PACKET_SIZE(packetLength);    // add CRC size
                   packetDataPointer = (uint8_t *)(&(currentDataEntry->data)+4);

                   if (packetLength+1 != *(uint8_t *)(&(currentDataEntry->data)+2)) {
                       printf("Size mismatch: %d %d\n", packetLength, *(uint8_t *)(&(currentDataEntry->data)+2));
                       printf("Data Start: 0x%x 0x%x 0x%x\n", packetDataPointer[0], packetDataPointer[1], packetDataPointer[2]);
                   }

                   lastRssi = rxStatistics.lastRssi; // TODO: save rssi only if frame was addressed to this node
                   printHex("RX: ", packetDataPointer, packetLength);
                   _rfDataLinkLayer.frameBytesReceived(packetDataPointer, packetLength);
                   RFQueue_nextEntry();
                }
                _loopState = RX_START;
            }
        }
        break;
    }
}

#endif
